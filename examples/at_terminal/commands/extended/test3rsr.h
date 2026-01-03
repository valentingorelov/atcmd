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

#ifndef ATCMD_TEST3RSR_H
#define ATCMD_TEST3RSR_H

#include <atcmd/server/extendedcommand.h>

struct Test3rsr : public atcmd::server::ExtendedCommand
{
	// Test command
	// Accepts 3 optional parameters: hexstring, string and hexstring
	struct Definition
	{
		static constexpr char name[] = "TEST3_RSR";

		struct Hexstring1 : public HexadecimalStringParameter
		{
			static constexpr bool is_optional = true;
			static constexpr uint8_t default_value[] = {0x01, 0x02};
			static constexpr uint16_t max_size = 20;
		};

		struct String : public StringParameter
		{
			static constexpr bool is_optional = true;
			static constexpr uint16_t max_length = 20;
			static constexpr const char* default_value = "abc";
		};

		struct Hexstring2 : public HexadecimalStringParameter
		{
			static constexpr bool is_optional = true;
			static constexpr uint8_t default_value[] = {0x03, 0x04};
			static constexpr uint16_t max_size = 20;
		};

		using Parameters = ParameterList<Hexstring1, String, Hexstring2>;

		static atcmd::RESULT_CODE onWrite(WriteServerHandle server_handle);
		static atcmd::RESULT_CODE onRead(ReadServerHandle server_handle);
		static const char* onTest(TestServerHandle server_handle);
	};
};

#endif // ATCMD_TEST3RSR_H
