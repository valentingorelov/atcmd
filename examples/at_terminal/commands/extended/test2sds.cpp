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

#include "test2sds.h"

#include <array>
#include <cstring>

static std::array<char, 21> l_s1;
static uint32_t l_d;
static std::array<char, 21> l_s2;


atcmd::RESULT_CODE Test2sds::Definition::onWrite(WriteServerHandle server_handle)
{
	Parameters parameters(server_handle);
	std::strncpy(l_s1.data(), parameters.getString<String1>(), l_s1.size());
	l_d = parameters.getNumeric<Decimal>();
	std::strncpy(l_s2.data(), parameters.getString<String2>(), l_s2.size());
	return atcmd::RESULT_CODE::OK;
}

atcmd::RESULT_CODE Test2sds::Definition::onRead(ReadServerHandle server_handle)
{
	server_handle.makeParameterInformationText<Parameters>(name)
			.printStringParameter<String1>(l_s1.data())
			.printNumericParameter<Decimal>(l_d)
			.printStringParameter<String2>(l_s2.data());
	return atcmd::RESULT_CODE::OK;
}

const char* Test2sds::Definition::onTest(TestServerHandle /*server_handle*/)
{
	return name;
}
