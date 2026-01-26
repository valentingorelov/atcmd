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

#ifndef ASYNCIOEMULATOR_H
#define ASYNCIOEMULATOR_H

#include "asyncworker.h"

#include <atomic>
#include <future>
#include <thread>
#include <cstdint>

struct AsyncIoEmulator : public AsyncWorker
{
	AsyncIoEmulator(std::mutex& mutex, std::condition_variable& cond);

	void write(uint_fast8_t index, uint32_t data);
	void writeTerminate();

	void read();
	void get(uint32_t& d0, uint32_t& d1, uint32_t& d2) const;

	void poll();

private:
	void write_(uint_fast8_t index, uint32_t data);
	void read_(std::stop_token stop_token);

	mutable std::mutex m_data_mutex;
	uint32_t m_d[3];

	std::atomic_bool  m_write_ongoing;
	std::atomic_bool  m_write_done;
	std::atomic_bool  m_write_terminated;
	std::future<void> m_write_future;

	std::atomic_bool  m_read_done;
	std::jthread      m_read_thread;
};

#endif // ASYNCIOEMULATOR_H
