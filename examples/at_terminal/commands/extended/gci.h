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

#ifndef GCI_H
#define GCI_H

#include <atcmd/server/extendedcommand.h>

struct Gci : public atcmd::server::ExtendedCommand
{
	// Country of installation
	struct Definition
	{
		static constexpr char name[] = "GCI";

		struct CountryCode : public HexadecimalNumericParameter
		{
			static constexpr bool is_optional = false;
			static constexpr Range ranges[] = {{0, 255}};
		};

		using Parameters = ParameterList<CountryCode>;

		static atcmd::RESULT_CODE onWrite(WriteServerHandle server_handle);
		static atcmd::RESULT_CODE onRead(ReadServerHandle server_handle);
		static const char* onTest(TestServerHandle server_handle);
	};
};

#endif // GCI_H
