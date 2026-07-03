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

#include "hwserial.h"

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hwserial, CONFIG_HWSERIAL_LOG_LEVEL);

HwSerial::HwSerial(const device& uart_dev, k_pipe& rx_pipe, k_pipe& tx_pipe, k_poll_event& tx_event) :
    Serial{rx_pipe, tx_pipe, tx_event},
    m_uart_dev{uart_dev}
{
    int r = uart_irq_callback_user_data_set(&m_uart_dev, usartIrqCb, this);
    if (r != 0)
    {
        LOG_ERR("Can not initialize UART RX. Errcode: %d", r);

        return;
    }
    uart_irq_rx_enable(&m_uart_dev);
}

void HwSerial::processTxData()
{
    if (m_tx_event.state == K_POLL_STATE_PIPE_DATA_AVAILABLE)
    {
        uart_irq_tx_enable(&m_uart_dev);
    }
}

void HwSerial::usartIrqCb(const device* uart_dev, void* user_data)
{
    uart_irq_update(uart_dev);

    uint8_t data;
    HwSerial& instance = *static_cast<HwSerial*>(user_data);

    if (uart_irq_rx_ready(uart_dev))
    {
        while (uart_fifo_read(uart_dev, &data, 1))
        {
            k_pipe_write(&instance.m_rx_pipe, &data, 1, K_NO_WAIT);
        }
    }

    if (uart_irq_tx_ready(uart_dev))
    {
        if (k_pipe_read(&instance.m_tx_pipe, &data, 1, K_NO_WAIT) > 0)
        {
            uart_fifo_fill(uart_dev, &data, 1);
        }
        else
        {
            uart_irq_tx_disable(uart_dev);
        }
    }
}
