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

#include "mv18am.h"

#include <array>
#include <cstring>

#include <atcmd/server/server_base.h>

static std::array<char, 101> l_data;

atcmd::RESULT_CODE Mv18am::Definition::onWrite(WriteServerHandle server_handle)
{
	Parameters parameters(server_handle);
	const char* v = parameters.getString<Message>();
	std::strncpy(l_data.data(), v, l_data.size());
	return atcmd::RESULT_CODE::OK;
}

atcmd::RESULT_CODE Mv18am::Definition::onRead(ReadServerHandle server_handle)
{
	server_handle.makeParameterInformationText<Parameters>(name)
			.printStringParameter<Message>(l_data.data());
	return atcmd::RESULT_CODE::OK;
}

const char* Mv18am::Definition::onTest(TestServerHandle server_handle)
{
	auto& server = server_handle.getServer();

	server.printInformationTextHeader();
	server.printExtendedInformationTextHeader(name);
	server.printText("100");
	server.printInformationTextTrailer();

	return nullptr;
}
