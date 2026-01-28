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

#include "test4async.h"
#include "../../asyncioemulator.h"

static uint8_t l_write_index = 0;

atcmd::RESULT_CODE Test4async::Definition::onWrite(WriteServerHandle server_handle)
{
	Parameters parameters(server_handle);
	AsyncIoEmulator* io = reinterpret_cast<AsyncIoEmulator*>(server_handle.getContext());

	switch (server_handle.getCallType()) {
	case WriteServerHandle::CALL_TYPE::REQUEST:
		l_write_index = 0;
		io->write(0, parameters.getNumeric<Param0>());
		return atcmd::RESULT_CODE::ASYNC;
	case WriteServerHandle::CALL_TYPE::ABORT:
		io->writeTerminate();
		return atcmd::RESULT_CODE::ERROR;
	case WriteServerHandle::CALL_TYPE::RESPONSE:
		if (l_write_index == 2)
		{
			return atcmd::RESULT_CODE::OK;
		}
		else
		{
			l_write_index++;
			switch (l_write_index) {
			case 1:
				io->write(1, parameters.getNumeric<Param1>());
				break;
			case 2:
				io->write(2, parameters.getNumeric<Param2>());
				break;
			default:
				break;
			}
			return atcmd::RESULT_CODE::ASYNC;
		}
	default:
		return atcmd::RESULT_CODE::ERROR;
	}
}

atcmd::RESULT_CODE Test4async::Definition::onRead(ReadServerHandle server_handle)
{
	AsyncIoEmulator* io = reinterpret_cast<AsyncIoEmulator*>(server_handle.getContext());

	switch (server_handle.getCallType()) {
	case WriteServerHandle::CALL_TYPE::REQUEST:
		io->read();
		return atcmd::RESULT_CODE::ASYNC;
	case WriteServerHandle::CALL_TYPE::ABORT:
		return atcmd::RESULT_CODE::ASYNC;
	case WriteServerHandle::CALL_TYPE::RESPONSE:
	{
		uint32_t d0;
		uint32_t d1;
		uint32_t d2;
		io->get(d0, d1, d2);
		server_handle.makeParameterInformationText<Parameters>(name)
				.printNumericParameter<Param0>(d0)
				.printNumericParameter<Param1>(d1)
				.printNumericParameter<Param2>(d2);
		return atcmd::RESULT_CODE::OK;
	}
	default:
		return atcmd::RESULT_CODE::ERROR;
	}
}

const char* Test4async::Definition::onTest(TestServerHandle /*server_handle*/)
{
	return name;
}
