/**
 * Copyright © 2026 Valentin Gorelov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the “Software”), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @brief
 * @author Valentin Gorelov <gorelov.valentin@gmail.com>
 */

#include "bleserial.h"

#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/logging/log.h>
#include <zephyr/net_buf.h>

#include <algorithm>

LOG_MODULE_REGISTER(bleserial, CONFIG_BLESERIAL_LOG_LEVEL);

/****************************** UART Service IDs ******************************/
/* Service UUID 6E400001-B5A3-F393-­E0A9-­E50E24DCCA9E */
// See: https://learn.adafruit.com/introducing-adafruit-ble-bluetooth-low-energy-friend/uart-service
static const struct bt_uuid_128 uart_uuid =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x6E400001, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E));

/* TX characteristic UUID 0x0002 */
static const struct bt_uuid_128 tx_uuid =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x6E400002, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E));

/* RX characteristic UUID 0x0003 */
static const struct bt_uuid_128 rx_uuid =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x6E400003, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E));

/****************************** BLE ******************************/
static const struct bt_data ad_uart[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    // UART service ID: 6E400001-B5A3-F393-­E0A9-­E50E24DCCA9E
    // TODO: remove duplicate
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, 0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01,
                  0x00, 0x40, 0x6E),
};

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data sd_uart[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, DEVICE_NAME_LEN),
};

static BleSerial* l_server_instance;
extern const struct bt_gatt_attr* rx_attr;

static uint8_t simulate_vnd;
static void vnd_ccc_cfg_changed(const struct bt_gatt_attr* attr, uint16_t value)
{
    simulate_vnd = (value == BT_GATT_CCC_INDICATE) ? 1 : 0;
}

BT_GATT_SERVICE_DEFINE(uart_svc, BT_GATT_PRIMARY_SERVICE(&uart_uuid),
                       BT_GATT_CHARACTERISTIC(&tx_uuid.uuid, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL,
                                              BleSerial::writeCb, NULL),
                       BT_GATT_CHARACTERISTIC(&rx_uuid.uuid, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ,
                                              BleSerial::readCb, NULL, NULL),
                       BT_GATT_CCC(vnd_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), );

const struct bt_gatt_attr* rx_attr = &uart_svc.attrs[3];

/* Handling BLE connection and disconnection */
/* Advertising should be reenabled after a disconnect event */
static void readvertise(struct k_work* work)
{
    int err;

    bt_le_adv_stop();
    err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad_uart, ARRAY_SIZE(ad_uart), sd_uart, ARRAY_SIZE(sd_uart));
    if (err)
    {
        LOG_ERR("Advertising failed to start. Errcode: %d", err);
        return;
    }
}
K_WORK_DELAYABLE_DEFINE(readvertise_work, readvertise);

static void connected(struct bt_conn* conn, uint8_t err)
{
    if (err)
    {
        LOG_DBG("Connection failed. Errcode: %d. Error description: %s", err, bt_hci_err_to_str(err));
    }
    else
    {
        LOG_DBG("Connected");
    }
}

static void disconnected(struct bt_conn* conn, uint8_t reason)
{
    LOG_DBG("Disconnected, reason 0x%02x %s", reason, bt_hci_err_to_str(reason));
    k_work_reschedule(&readvertise_work, K_NO_WAIT);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

/* Handling BLE TX */
NET_BUF_POOL_DEFINE(ble_tx_pool, 2, CONFIG_BT_L2CAP_TX_MTU - 3, 0, NULL);
static net_buf* current_tx_buf;
K_MUTEX_DEFINE(current_tx_buf_mutex);

static void ble_send(struct k_work* work)
{
    auto tx_data = [](struct bt_conn* conn, void*)
    {
        k_mutex_lock(&current_tx_buf_mutex, K_FOREVER);
        net_buf* tx_buf = current_tx_buf;
        current_tx_buf = nullptr;
        k_mutex_unlock(&current_tx_buf_mutex);

        std::size_t mtu = bt_gatt_get_mtu(conn) - 3;
        uint8_t* data = tx_buf->data;
        std::size_t len = tx_buf->len;
        while (len != 0)
        {
            std::size_t tx_sz = std::min(mtu, len);
            LOG_DBG("BLE TX: sending %d bytes", tx_sz);
            int r = bt_gatt_notify(conn, rx_attr, tx_buf->data, tx_sz);
            if (r != 0)
            {
                LOG_ERR("Can not send BLE data. Errcode: %d", r);
                return;
            }
            else
            {
                LOG_DBG("BLE TX: %d bytes sent", tx_sz);
                data += tx_sz;
                len -= tx_sz;
            }
        }

        net_buf_unref(tx_buf);
    };

    bt_conn_foreach(BT_CONN_TYPE_LE, tx_data, nullptr);
}
K_WORK_DELAYABLE_DEFINE(send_work, ble_send);

BleSerial::BleSerial(AtServer& at_server, k_pipe& rx_pipe, k_pipe& tx_pipe, k_poll_event& tx_event) :
    Serial{rx_pipe, tx_pipe, tx_event},
    m_at_server{at_server}
{
    char addr_s[BT_ADDR_LE_STR_LEN];
    struct bt_le_oob oob;

    l_server_instance = this;

    m_at_server.getCommunicationParameters().setEchoEnabled(false);

    int r = bt_enable(NULL);
    if (r != 0)
    {
        LOG_ERR("Can not initialize BLE. Errcode: %d", r);

        return;
    }

    LOG_DBG("Bluetooth initialized");

    /* Start advertising */
    r = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad_uart, ARRAY_SIZE(ad_uart), sd_uart, ARRAY_SIZE(sd_uart));
    if (r)
    {
        printk("Advertising failed to start. Errcode: %d", r);
        return;
    }

    /* Print the advertising address */
    bt_le_oob_get_local(BT_ID_DEFAULT, &oob);
    bt_addr_le_to_str(&oob.addr, addr_s, sizeof(addr_s));
    LOG_DBG("Initial advertising as %s", addr_s);
}

void BleSerial::processTxData()
{
    auto tx_data = [](struct bt_conn* conn, void* data)
    {
        k_mutex_lock(&current_tx_buf_mutex, K_FOREVER);
        if (current_tx_buf == nullptr)
        {
            current_tx_buf = net_buf_alloc(&ble_tx_pool, K_FOREVER);
        }

        std::size_t tailroom = net_buf_tailroom(current_tx_buf);
        std::size_t mtu = bt_gatt_get_mtu(conn) - 3;
        std::size_t tx_size = std::min(tailroom, mtu);

        BleSerial* server = reinterpret_cast<BleSerial*>(data);
        auto& cp = server->m_at_server.getCommunicationParameters();
        uint8_t ch;

        k_work_schedule(&send_work, K_MSEC(CONFIG_BLESERIAL_TX_TIMEOUT));
        while (k_pipe_read(&server->m_tx_pipe, &ch, 1, K_NO_WAIT) > 0)
        {
            net_buf_add_u8(current_tx_buf, ch);
            tx_size--;
            bool flush =
                (tx_size == 0) || (ch == cp.getCmdLineTerminationChar()) || (ch == cp.getResponseFormattingChar());
            if (flush)
            {
                LOG_DBG("BLE TX flushed");
                k_work_reschedule(&send_work, K_NO_WAIT);
            }
        }

        k_mutex_unlock(&current_tx_buf_mutex);
    };

    if (m_tx_event.state == K_POLL_STATE_PIPE_DATA_AVAILABLE)
    {
        bt_conn_foreach(BT_CONN_TYPE_LE, tx_data, this);
    }
}

ssize_t BleSerial::readCb(struct bt_conn* conn, const struct bt_gatt_attr* attr, void* buf, uint16_t len,
                          uint16_t offset)
{
    LOG_DBG("Read callback called");

    return 0;
}

ssize_t BleSerial::writeCb(struct bt_conn* conn, const struct bt_gatt_attr* attr, const void* buf, uint16_t len,
                           uint16_t offset, uint8_t flags)
{
    LOG_DBG("Write callback called");
    k_pipe_write(&l_server_instance->m_rx_pipe, reinterpret_cast<const uint8_t*>(buf), len, K_NO_WAIT);
    return len;
}
