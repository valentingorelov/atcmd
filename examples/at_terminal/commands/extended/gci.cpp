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

#include "gci.h"

uint32_t l_gci;

atcmd::RESULT_CODE Gci::Definition::onWrite(WriteServerHandle server_handle)
{
	Parameters parameters(server_handle);
	l_gci = parameters.getNumeric<CountryCode>();
	return atcmd::RESULT_CODE::OK;
}

atcmd::RESULT_CODE Gci::Definition::onRead(ReadServerHandle server_handle)
{
	// server_handle.makeParameterInformationText(name)
	// 		.printNumericParameter(m_gci, 16);
	server_handle.makeParameterInformationText<Parameters>(name)
			.printNumericParameter<CountryCode>(l_gci);
	return atcmd::RESULT_CODE::OK;
}

const char* Gci::Definition::onTest(TestServerHandle /*server_handle*/)
{
	return name;
}
