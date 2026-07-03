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

#ifndef BLESERIAL_H
#define BLESERIAL_H

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>

#include "atserver.h"
#include "serial.h"

struct BleSerial : public Serial
{
    BleSerial(AtServer& at_server, k_pipe& rx_pipe, k_pipe& tx_pipe, k_poll_event& tx_event);

    void processTxData();

    static ssize_t readCb(struct bt_conn* conn, const struct bt_gatt_attr* attr, void* buf, uint16_t len,
                          uint16_t offset);
    static ssize_t writeCb(struct bt_conn* conn, const struct bt_gatt_attr* attr, const void* buf, uint16_t len,
                           uint16_t offset, uint8_t flags);

private:
    AtServer& m_at_server;
};

#endif // BLESERIAL_H
