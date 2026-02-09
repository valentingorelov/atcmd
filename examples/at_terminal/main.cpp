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

#include <string>

#include "server_config.h"
#include "asyncinput.h"
#include "asyncioemulator.h"

void execCmd(const std::string& cmd)
{
	for (const char& c : cmd)
	{
		server.feed(c, true);
	}
}

int main()
{
	std::condition_variable l_cond;
	std::mutex l_mutex;
	std::unique_lock lock(l_mutex);

	AsyncInput input(l_mutex, l_cond);
	std::string cmd;

	AsyncIoEmulator emulator(l_mutex, l_cond);
	server.setContext(&emulator);
	server.getCommunicationParameters().setEchoEnabled(false);

	while (true)
	{
		l_cond.wait(lock);
		cmd = input.getLine();
		if (!cmd.empty())
		{
			execCmd(cmd);
		}
		emulator.poll();
	}

	return 0;
}
