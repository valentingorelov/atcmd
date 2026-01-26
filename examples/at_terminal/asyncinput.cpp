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

#include "asyncinput.h"

#include <iostream>

AsyncInput::AsyncInput(std::mutex& mutex, std::condition_variable& cond) :
	AsyncWorker(mutex, cond)
{
	start();
}

std::string AsyncInput::getLine()
{
	std::string r;
	if (m_ready)
	{
		r = m_line.get();
		start();
	}
	return r;
}

void AsyncInput::start()
{
	m_ready = false;
	m_line = std::async(std::launch::async, &AsyncInput::getLine_, this);
}

std::string AsyncInput::getLine_()
{
	std::string cmd;
	std::getline(std::cin, cmd);

	std::lock_guard lock(m_mutex);
	m_ready = true;
	m_cond.notify_all();
	return cmd;
}
