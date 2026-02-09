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
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <atcmd/server/server.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

K_PIPE_DEFINE(rx_buf, 100, 1);

static void printChar(char ch, void* /*context*/)
{
	printk("%c", ch);
}

struct ServerSettings
{
	using BasicCommands = atcmd::server::BasicCommandList<>;
	using AmpersandCommands = atcmd::server::AmpersandCommandList<>;
	using ExtendedCommands = atcmd::server::ExtendedCommandList<>;

	static constexpr std::size_t max_commands_per_line = 3;
};

static atcmd::server::Server<ServerSettings> server(printChar);

static void usart_irq_cb(const struct device *dev, void *user_data)
{
	uart_irq_update(dev);

	if (uart_irq_rx_ready(dev))
	{
		uint8_t data;
		while (uart_fifo_read(dev, &data, 1))
		{
			k_pipe_write(&rx_buf, &data, 1, K_NO_WAIT);
		}
	}
}

int main(void)
{
	//server.getCommunicationParameters().setCmdLineTerminationChar('\n');

	uart_irq_callback_user_data_set(uart_dev, usart_irq_cb, (void *)uart_dev);
	uart_irq_rx_enable(uart_dev);

	uint8_t data;
	while (k_pipe_read(&rx_buf, &data, 1, K_FOREVER))
	{
		server.feed(static_cast<char>(data), false);
	}

	return 0;
}
