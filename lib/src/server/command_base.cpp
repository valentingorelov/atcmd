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

#include <atcmd/server/command_base.h>
#include <atcmd/server/server_base.h>

namespace atcmd::server::detail {

Server& CommandBase::ServerHandle::getServer()
{
	return m_server;
}

Command::ServerHandle::ServerHandle(Server& server, bool is_last_command) :
	m_server{server},
	m_is_last_command{is_last_command}
{}

Command::ServerHandle::InformationText::InformationText(Server& server, bool is_result_code) :
	m_server{server},
	m_is_result_code{is_result_code}
{
	if (m_is_result_code)
	{
		m_server.printResultCodeHeader();
	}
	else
	{
		m_server.printInformationTextHeader();
	}
}

Command::ServerHandle::InformationText::~InformationText()
{
	if (m_is_result_code)
	{
		m_server.printResultCodeTrailer();
	}
	else
	{
		m_server.printInformationTextTrailer();
	}
}

void Command::ServerHandle::InformationText::printText(const char* text) &&
{
	m_server.printText(text);
}

Command::ParamServerHandle::ParamServerHandle(const uint8_t* param_start) :
	m_param_start{param_start}
{}

const uint8_t* Command::ParamServerHandle::getParamStart() const
{
	return m_param_start;
}

Command::ParameterListBase::ParameterListBase(const uint8_t* params) :
	m_params{params}
{}

uint32_t Command::ParameterListBase::getNumeric_(std::size_t offset) const
{
	uint32_t r = 0;
	for (uint_fast8_t i = 0; i < sizeof(r); i++)
	{
		r |= m_params[offset + i] << (8 * i);
	}
	return r;
}

} /* atcmd::server::detail */
