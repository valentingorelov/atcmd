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

#include "test3rsr.h"

#include <array>
#include <cstring>

static std::array<uint8_t, 20> l_r1;
static std::size_t l_r1_size;

static std::array<char, 21> l_s;

static std::array<uint8_t, 20> l_r2;
static std::size_t l_r2_size;

atcmd::RESULT_CODE Test3rsr::Definition::onWrite(WriteServerHandle server_handle)
{
	Parameters parameters(server_handle);

	auto r1 = parameters.getHexString<Hexstring1>();
	std::copy(r1.begin(), r1.end(), l_r1.begin());
	l_r1_size = r1.size();

	std::strncpy(l_s.data(), parameters.getString<String>(), l_s.size());

	auto r2 = parameters.getHexString<Hexstring2>();
	std::copy(r2.begin(), r2.end(), l_r2.begin());
	l_r2_size = r2.size();

	return atcmd::RESULT_CODE::OK;
}

atcmd::RESULT_CODE Test3rsr::Definition::onRead(ReadServerHandle server_handle)
{
	server_handle.makeParameterInformationText<Parameters>(name)
			.printHexadecimalStringParameter<Hexstring1>(l_r1.data(), l_r1_size)
			.printStringParameter<String>(l_s.data())
			.printHexadecimalStringParameter<Hexstring2>(l_r2.data(), l_r2_size);
	return atcmd::RESULT_CODE::OK;
}

const char* Test3rsr::Definition::onTest(TestServerHandle /*server_handle*/)
{
	return name;
}
