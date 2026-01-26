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

#include "asyncioemulator.h"
#include "server_config.h"
#include "commands/extended/test4async.h"

#include <thread>
#include <chrono>
#include <functional>

AsyncIoEmulator::AsyncIoEmulator(std::mutex& mutex, std::condition_variable& cond) :
	AsyncWorker(mutex, cond),
	m_d{},
	m_write_ongoing{false},
	m_write_done{false},
	m_write_terminated{false},
	m_read_done{false},
	m_read_thread(std::bind_front(&AsyncIoEmulator::read_, this))
{}

void AsyncIoEmulator::write(uint_fast8_t index, uint32_t data)
{
	if (m_write_ongoing)
	{
		return;
	}
	m_write_ongoing = true;
	m_write_done = false;
	m_write_terminated = false;
	m_write_future = std::async(std::launch::async, &AsyncIoEmulator::write_, this, index, data);
}

void AsyncIoEmulator::writeTerminate()
{
	m_write_terminated = true;
}

void AsyncIoEmulator::read()
{
	m_read_done = false;
}

void AsyncIoEmulator::get(uint32_t& d0, uint32_t& d1, uint32_t& d2) const
{
	std::lock_guard lock(m_data_mutex);
	d0 = m_d[0];
	d1 = m_d[1];
	d2 = m_d[2];
}

void AsyncIoEmulator::poll()
{
	if (m_write_done)
	{
		m_write_ongoing = false;
		m_write_done = false;
		server.onExtendedCommandWriteUpdate<Test4async>();
	}
	if (m_read_done)
	{
		server.onExtendedCommandReadUpdate<Test4async>();
	}
}

void AsyncIoEmulator::write_(uint_fast8_t index, uint32_t data)
{
	std::lock_guard lock(m_data_mutex);

	for (std::size_t i = 0; i < 10; i++)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (m_write_terminated)
		{
			return;
		}
	}
	m_d[index] = data;

	std::lock_guard lock2(m_mutex);
	m_write_done = true;
	m_cond.notify_all();
}

void AsyncIoEmulator::read_(std::stop_token stop_token)
{
	while (!stop_token.stop_requested())
	{
		std::this_thread::sleep_for(std::chrono::seconds(3));

		std::lock_guard lock2(m_mutex);
		m_read_done = true;
		m_cond.notify_all();
	}
}
