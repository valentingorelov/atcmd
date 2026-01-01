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

#include <utility>

#include <atcmd/server/extendedcommand.h>
#include <atcmd/server/server_base.h>

namespace atcmd::server::detail {

static void emptyCharCb(char /*ch*/, void* /*user_handle*/)
{
}

ExtendedCommandBase::TestServerHandle::ExtendedInformationText::ExtendedInformationText(Server& server, bool is_result_code, const char* name, bool is_silent) :
	InformationText(server, is_result_code),
	m_print_char_callback{m_server.getPrintCharCallback()}
{
	if (is_silent)
	{
		m_server.setPrintCharCallback(emptyCharCb);
	}
	m_server.printExtendedInformationTextHeader(name);
}

ExtendedCommandBase::TestServerHandle::ExtendedInformationText::~ExtendedInformationText()
{
	m_server.setPrintCharCallback(m_print_char_callback);
}

ExtendedCommandBase::TestServerHandle::TestServerHandle(Server& server, bool is_last_command) :
	ServerHandle(server, is_last_command)
{

}

ExtendedCommandBase::WriteServerHandle::WriteServerHandle(const uint8_t* param_start, Server& server, bool is_last_command) :
	TestServerHandle(server, is_last_command),
	ParamServerHandle(param_start)
{}

ExtendedCommandBase::ReadServerHandle::ParameterInformationText::ParameterInformationText(Server& server, bool is_result_code, const char* name, bool is_silent) :
	ExtendedInformationText(server, is_result_code, name, is_silent)
{}

ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond
ExtendedCommandBase::ReadServerHandle::ParameterInformationText::printNumericParameter(uint32_t value, uint8_t base) &&
{
	printNumericParameter_(value, base);
	return ParameterInformationTextSecond(m_server);
}

ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond
ExtendedCommandBase::ReadServerHandle::ParameterInformationText::printStringParameter(const char* s) &&
{
	printStringParameter_(s);
	return ParameterInformationTextSecond(m_server);
}

ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond
ExtendedCommandBase::ReadServerHandle::ParameterInformationText::printHexadecimalStringParameter(const uint8_t* data, uint16_t size) &&
{
	printHexadecimalStringParameter_(data, size);
	return ParameterInformationTextSecond(m_server);
}

void ExtendedCommandBase::ReadServerHandle::ParameterInformationText::printNumericParameter_(uint32_t value, uint8_t base)
{
	m_server.printNumber(value, base);
}

void ExtendedCommandBase::ReadServerHandle::ParameterInformationText::printStringParameter_(const char* s)
{
	m_server.printString(s);
}

void ExtendedCommandBase::ReadServerHandle::ParameterInformationText::printHexadecimalStringParameter_(const uint8_t* data, uint16_t size)
{
	m_server.printHexadecimalString(data, size);
}

ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond::ParameterInformationTextSecond(Server& server) :
	m_server{server}
{}

ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond&&
ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond::printNumericParameter(uint32_t value, uint8_t base) &&
{
	printNumericParameter_(value, base);
	return std::move(*this);
}

ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond&&
ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond::printStringParameter(const char* s) &&
{
	printStringParameter_(s);
	return std::move(*this);
}

ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond&&
ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond::printHexadecimalStringParameter(const uint8_t* data, uint16_t size) &&
{
	printHexadecimalStringParameter_(data, size);
	return std::move(*this);
}

void ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond::printNumericParameter_(uint32_t value, uint8_t base)
{
	m_server.printChar(',');
	m_server.printNumber(value, base);
}

void ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond::printStringParameter_(const char* s)
{
	m_server.printChar(',');
	m_server.printString(s);
}

void ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecond::printHexadecimalStringParameter_(const uint8_t* data, uint16_t size)
{
	m_server.printChar(',');
	m_server.printHexadecimalString(data, size);
}

ExtendedCommandBase::ReadServerHandle::ReadServerHandle(Server& server, bool is_last_command) :
	TestServerHandle(server, is_last_command)
{}

ExtendedCommandBase::ReadServerHandle::ParameterInformationText
ExtendedCommandBase::ReadServerHandle::makeParameterInformationText(const char* name, bool is_result_code)
{
	return ParameterInformationText(m_server, is_result_code, name, !m_is_last_command);
}

} /* namespace atcmd::server::detail */
