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

#ifndef ATCMD_SERVER_BASE_H
#define ATCMD_SERVER_BASE_H

#include <atcmd/common.h>
#include <atcmd/server/basiccommand.h>
#include <atcmd/server/extendedcommand.h>
#include <atcmd/server/sparameters.h>

namespace atcmd::server::detail {

class Server
{
protected:
	enum class CMD_TYPE : uint8_t
	{
		READ,
		WRITE,
		TEST
	};

public:
	SParameters& getCommunicationParameters();

	void printChar(char ch);
	void printText(const char* text);
	void printNumber(uint32_t number, uint8_t base);
	void printString(const char* string);
	void printHexadecimalString(const uint8_t* data, uint16_t size);

	void printInformationTextHeader();
	void printInformationTextTrailer();

	void printExtendedInformationTextHeader(const char* name);

	void printResultCodeHeader();
	void printResultCodeTrailer();
	void printResultCode(RESULT_CODE code);

	PrintCharCallback getPrintCharCallback();
	void setPrintCharCallback(PrintCharCallback print_char_callback);

	void setContext(void* context);
	void* getContext();

protected:
	Server(PrintCharCallback print_char_callback, void* context);

	BasicCommandBase::BasicServerHandle getBasicHandle(const uint8_t* param_start, bool is_last_command);

	ExtendedCommandBase::ReadServerHandle getReadHandle(bool is_last_command);
	ExtendedCommandBase::WriteServerHandle getWriteHandle(const uint8_t* param_start, bool is_last_command);
	ExtendedCommandBase::TestServerHandle getTestHandle(bool is_last_command);

private:
	PrintCharCallback m_print_char_callback;
	void* m_context;

	SParameters m_s_parameters;
};

} /* namespace atcmd::server::detail */

#endif // ATCMD_SERVER_BASE_H
