/**
* Copyright © 2025 Valentin Gorelov
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

#include <zephyr/kernel.h>

#include "atserver.h"
#include "bleserial.h"
#include "hwserial.h"

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const device* const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

K_PIPE_DEFINE(uart_rx_pipe, 128, 1);
K_PIPE_DEFINE(uart_tx_pipe, 128, 1);

K_PIPE_DEFINE(ble_rx_pipe, 128, 1);
K_PIPE_DEFINE(ble_tx_pipe, 128, 1);

static struct k_poll_event events[4] = {
    K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_PIPE_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &uart_rx_pipe, 0),
    K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_PIPE_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &uart_tx_pipe, 0),
    K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_PIPE_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &ble_rx_pipe, 0),
    K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_PIPE_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &ble_tx_pipe, 0),
};

int main(void)
{
    AtServer server_serial(uart_rx_pipe, events[0], uart_tx_pipe);
    HwSerial hw_serial(*uart_dev, uart_rx_pipe, uart_tx_pipe, events[1]);

    AtServer server_ble(ble_rx_pipe, events[2], ble_tx_pipe);
    BleSerial hw_ble(server_ble, ble_rx_pipe, ble_tx_pipe, events[3]);

	while (true)
	{
		int rc = k_poll(events, ARRAY_SIZE(events), K_FOREVER);
		if (rc == 0)
		{
            server_serial.processRxData();
            hw_serial.processTxData();

            server_ble.processRxData();
            hw_ble.processTxData();
        }
        events[0].state = K_POLL_STATE_NOT_READY;
        events[1].state = K_POLL_STATE_NOT_READY;
        events[2].state = K_POLL_STATE_NOT_READY;
        events[3].state = K_POLL_STATE_NOT_READY;
    }

	return 0;
}
