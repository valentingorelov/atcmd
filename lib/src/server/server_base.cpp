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

#include <atcmd/server/server_base.h>

#include <atcmd/detail/characters.h>

namespace atcmd::server::detail {

SParameters& Server::getCommunicationParameters()
{
	return m_s_parameters;
}

void Server::printChar(char ch)
{
	m_print_char_callback(ch, m_context);
}

void Server::printText(const char* text)
{
	while (*text != '\0')
	{
		printChar(*text++);
	}
}

void Server::printNumber(uint32_t number, uint8_t base)
{
	atcmd::detail::Characters::printNumber(number, base, m_print_char_callback, m_context);
}

void Server::printString(const char* string)
{
	printChar('"');
	printText(string);
	printChar('"');
}

void Server::printHexadecimalString(const uint8_t* data, uint16_t size)
{
	printChar('"');
	atcmd::detail::Characters::printHexadecimalString(data, size, m_print_char_callback, m_context);
	printChar('"');
}

void Server::printInformationTextHeader()
{
	printResultCodeHeader();
}

void Server::printInformationTextTrailer()
{
	printChar(m_s_parameters.getCmdLineTerminationChar());
	printChar(m_s_parameters.getResponseFormattingChar());
}

void Server::printExtendedInformationTextHeader(const char* name)
{
	printChar('+');
	printText(name);
	printChar(':');
}

void Server::printResultCodeHeader()
{
	if (getCommunicationParameters().isVerbose())
	{
		printChar(m_s_parameters.getCmdLineTerminationChar());
		printChar(m_s_parameters.getResponseFormattingChar());
	}
}

void Server::printResultCodeTrailer()
{
	printChar(m_s_parameters.getCmdLineTerminationChar());
	if (getCommunicationParameters().isVerbose())
	{
		printChar(m_s_parameters.getResponseFormattingChar());
	}
}

void Server::printResultCode(RESULT_CODE code)
{
	static constexpr const char* codes[] =
	{
		"OK",
		"CONNECT",
		"RING",
		"NO CARRIER",
		"ERROR",
		"",
		"NO DIALTONE",
		"BUSY",
		"NO ANSWER"
	};
	assert(code <= RESULT_CODE::NO_ANSWER);

	printResultCodeHeader();
	if (getCommunicationParameters().isVerbose())
	{
		printText(codes[static_cast<uint8_t>(code)]);
	}
	else
	{
		printChar('0' + static_cast<uint8_t>(code));
	}
	printResultCodeTrailer();
}

PrintCharCallback Server::getPrintCharCallback()
{
	return m_print_char_callback;
}

void Server::setPrintCharCallback(PrintCharCallback print_char_callback)
{
	m_print_char_callback = print_char_callback;
}

void Server::setContext(void* context)
{
	m_context = context;
}

void* Server::getContext()
{
	return m_context;
}

Server::Server(PrintCharCallback print_char_callback, void* context) :
	m_print_char_callback{print_char_callback},
	m_context{context}
{}

BasicCommandBase::BasicServerHandle Server::getBasicHandle(const uint8_t* param_start, bool is_last_command)
{
	return BasicCommandBase::BasicServerHandle(param_start, *this, is_last_command);
}

ExtendedCommandBase::ReadServerHandle Server::getReadHandle(bool is_last_command)
{
	return ExtendedCommandBase::ReadServerHandle(*this, is_last_command);
}

ExtendedCommandBase::WriteServerHandle Server::getWriteHandle(const uint8_t* param_start, bool is_last_command)
{
	return ExtendedCommandBase::WriteServerHandle(param_start, *this, is_last_command);
}

ExtendedCommandBase::TestServerHandle Server::getTestHandle(bool is_last_command)
{
	return ExtendedCommandBase::TestServerHandle(*this, is_last_command);
}

} /* namespace atcmd::server::detail */
