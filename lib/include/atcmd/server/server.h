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

#ifndef ATCMD_SERVER_H
#define ATCMD_SERVER_H

#include <cstdint>

#include <atcmd/detail/server_cmdline.h>
#include <atcmd/detail/characters.h>

namespace atcmd::server {

namespace detail
{

template<std::size_t ext_cmd_size, class ExtendedCommands>
struct ServerTrieHolder;

template<class ExtendedCommands>
struct ServerTrieHolder<0, ExtendedCommands>
{
	using m_trie = void;
};

template<std::size_t ext_cmd_size, class ExtendedCommands>
struct ServerTrieHolder
{
	typename ExtendedCommands::Trie m_trie;
};

} /* namespace detail */

template<concepts::ServerSettings Settings>
class Server :
		public detail::ServerCmdline<Settings>,
		public detail::ServerTrieHolder<Settings::ExtendedCommands::size, typename Settings::ExtendedCommands>
{
	using Base = detail::ServerCmdline<Settings>;

public:
	using Base::getCommunicationParameters;

	Server(PrintCharCallback print_char_callback, void* context = nullptr) :
		detail::ServerCmdline<Settings>(print_char_callback, context),
		m_state{&Server::stateA}
	{}

	void feed(char ch, bool abortable = false)
	{
		if constexpr (Settings::ExtendedCommands::size != 0)
		{
			if (m_state != &Server::stateExtendedParamString)
			{
				ch = atcmd::detail::Characters::toUpper(ch);
			}
		}
		else
		{
			ch = atcmd::detail::Characters::toUpper(ch);
		}
		(this->*m_state)(ch, abortable);
	}

	template<concepts::BasicCommand Cmd>
	void onBasicCommandExecUpdate()
	{
		static constexpr uint16_t cmd_id =
				Base::getBasicCmdOffset() + 1 + Settings::BasicCommands::template getCommandPosition<Cmd>();
		continueCmdExec(cmd_id);
	}

	template<concepts::AmpersandCommand Cmd>
	void onAmpersandCommandExecUpdate()
	{
		static constexpr uint16_t cmd_id =
				Base::getAmpersandCmdOffset() + Settings::AmpersandCommands::template getCommandPosition<Cmd>();
		continueCmdExec(cmd_id);
	}

	template<concepts::ExtendedCommand Cmd>
	void onExtendedCommandReadUpdate()
	{
		if constexpr (Settings::ExtendedCommands::size != 0)
		{
			static constexpr uint16_t cmd_id =
					Base::getExtCmdId(Settings::ExtendedCommands::template getCommandPosition<Cmd>(), CMD_TYPE::READ);
			continueCmdExec(cmd_id);
		}
	}

	template<concepts::ExtendedCommand Cmd>
	void onExtendedCommandWriteUpdate()
	{
		if constexpr (Settings::ExtendedCommands::size != 0)
		{
			static constexpr uint16_t cmd_id =
					Base::getExtCmdId(Settings::ExtendedCommands::template getCommandPosition<Cmd>(), CMD_TYPE::WRITE);
			continueCmdExec(cmd_id);
		}
	}

private:
	using Base::checkParameterBufferOvf;
	using Base::addBasicCmd;
	using Base::addExtCmd;
	using Base::addByteParameter;
	using Base::addNumericParameter;
	using Base::addChar;
	using Base::addByte;
	using Base::finalizeString;
	using Base::finalizeHexString;
	using Base::finalizeBasicCmd;
	using Base::finalizeExtCmd;
	using Base::abortCmdExec;

	using CMD_TYPE = Base::CMD_TYPE;
	using Base::m_param_value_num;
	using Base::m_param_string_size;
	using Base::m_param_hex_string;
	using Base::m_basic_cmd_index;
	using Base::m_param_index;

	// State machine states
	using State = void (Server::*)(char, bool);

	void stateA(char ch, bool /*abortable*/)
	{
		if (ch == ' ')
		{
			return;
		}
		if (ch == 'A')
		{
			m_state = &Server::stateT;
		}
	}

	void stateT(char ch, bool /*abortable*/)
	{
		if (ch == ' ')
		{
			return;
		}
		switch (ch) {
		case 'T':
			Base::resetCmdline();
			if constexpr (Settings::ExtendedCommands::size != 0)
			{
				m_trie.reset();
			}
			m_state = &Server::stateBody;
			break;
		case '/':
			startCmdExec();
			break;
		default:
			m_state = &Server::stateA;
			break;
		}
	}

	void stateBody(char ch, bool abortable = false)
	{
		(void)abortable;

		if (ch == ' ')
		{
			return;
		}
		if (atcmd::detail::Characters::isAlphabetic(ch))
		{
			if (ch == 'S')
			{
				m_param_value_num = 0;
				m_state = &Server::stateS;
			}
			else
			{
				if constexpr (Settings::BasicCommands::size != 0)
				{
					int_fast8_t cmd_index = Settings::BasicCommands::getCmdIndex(ch);
					if (cmd_index == -1)
					{
						// Unknown command
						m_state = &Server::stateError;
					}
					else if (!addBasicCmd(1 + cmd_index))
					{
						// Buffer overflow
						m_state = &Server::stateError;
					}
					else
					{
						const detail::BasicCmdDef& cmd = Settings::BasicCommands::m_cmd_defs[cmd_index];
						if (cmd.numeric_ranges == nullptr)
						{
							// This command does not support numeric parameters
							finalizeBasicCmd();
							m_state = &Server::stateBody;
						}
						else
						{
							m_basic_cmd_index = cmd_index;
							m_param_value_num = 0;
							m_state = &Server::stateBasicParameter;
						}
					}
				}
				else
				{
					m_state = &Server::stateError;
				}
			}
		}
		else
		{
			if (ch == '&')
			{
				if constexpr (Settings::AmpersandCommands::size != 0)
				{
					m_state = &Server::stateAmpersand;
				}
				else
				{
					m_state = &Server::stateError;
				}
			}
			else if (ch == '+')
			{
				if constexpr (Settings::ExtendedCommands::size != 0)
				{
					m_state = &Server::stateExtended;
				}
				else
				{
					m_state = &Server::stateError;
				}
			}
			else if (ch == getCommunicationParameters().getCmdLineTerminationChar())
			{
				startCmdExec();
			}
			else
			{
				// Unexpected character
				m_state = &Server::stateError;
			}
		}
	}

	void stateS(char ch, bool /*abortable*/)
	{
		if (ch == ' ')
		{
			return;
		}
		if ((ch == '=') || (ch == '?'))
		{
			if ((m_param_value_num < 3) || (m_param_value_num > 4))
			{
				// Unsupported S parameter
				m_state = &Server::stateError;
			}
			else if (!addBasicCmd(0))
			{
				// Buffer overflow
				m_state = &Server::stateError;
				return;
			}

			if (ch == '=')
			{
				m_param_value_num |= 0x80;
				m_state = &Server::stateSSet;
			}
			else
			{
				m_state = &Server::stateBody;
			}

			if (!addByteParameter())
			{
				// Buffer overflow
				m_state = &Server::stateError;
			}
			else
			{
				if (m_state == &Server::stateBody)
				{
					finalizeBasicCmd();
				}
				m_param_value_num = 0;
			}
		}
		else if (!processDecNumericChar(ch))
		{
			m_state = &Server::stateError;
			Server::stateError(ch);
		}
	}

	void stateSSet(char ch, bool /*abortable*/)
	{
		if (ch == ' ')
		{
			return;
		}
		if (!processDecNumericChar(ch))
		{
			if (m_param_value_num > 127)
			{
				// Unsupported S3 or S4 value
				m_state = &Server::stateError;
			}
			else if (!addByteParameter())
			{
				// Buffer overflow
				m_state = &Server::stateError;
			}
			else
			{
				finalizeBasicCmd();
				m_state = &Server::stateBody;
			}
			(this->*m_state)(ch, false);
		}
	}

	void stateAmpersand(char ch, bool /*abortable*/)
	{
		if (ch == ' ')
		{
			return;
		}
		int_fast8_t cmd_index = Settings::AmpersandCommands::getCmdIndex(ch);
		if (cmd_index == -1)
		{
			// Unknown command
			m_state = &Server::stateError;
		}
		else if (!addBasicCmd(1 + Settings::BasicCommands::size + cmd_index))
		{
			// Buffer overflow
			m_state = &Server::stateError;
		}
		else
		{
			const detail::BasicCmdDef& cmd = Settings::AmpersandCommands::m_cmd_defs[cmd_index];
			if (cmd.numeric_ranges == nullptr)
			{
				// This command does not support numeric parameters
				m_state = &Server::stateBody;
			}
			else
			{
				m_basic_cmd_index = cmd_index + 1 + Settings::BasicCommands::size;
				m_param_value_num = 0;
				m_state = &Server::stateBasicParameter;
			}
		}
	}

	void stateBasicParameter(char ch, bool /*abortable*/)
	{
		if (ch == ' ')
		{
			return;
		}
		if (atcmd::detail::Characters::isNumeric(ch))
		{
			if (!processDecNumericChar(ch))
			{
				// Buffer overflow
				m_state = &Server::stateError;
			}
		}
		else
		{
			// TODO optimize for fixed basic and ampersand command index offsets
			const detail::BasicCmdDef* cmd_def;
			static constexpr uint8_t basic_cmd_count = 1 + Settings::BasicCommands::size;
			if (m_basic_cmd_index >= basic_cmd_count)
			{
				if constexpr (Settings::AmpersandCommands::size != 0)
				{
					cmd_def = &Settings::AmpersandCommands::m_cmd_defs[m_basic_cmd_index - basic_cmd_count];
				}
			}
			else
			{
				if constexpr (Settings::BasicCommands::size != 0)
				{
					cmd_def = &Settings::BasicCommands::m_cmd_defs[m_basic_cmd_index];
				}
			}
			if (addNumericParameter(*cmd_def))
			{
				finalizeBasicCmd();
				m_state = &Server::stateBody;
				stateBody(ch);
			}
			else
			{
				// Range validation failure
				m_state = &Server::stateError;
			}
			(this->*m_state)(ch, false);
		}
	}

	void stateExtended(char ch, bool /*abortable*/)
	{
		if (ch == ' ')
		{
			return;
		}
		if (ch == '=')
		{
			// Either write or test
			m_state = &Server::stateExtendedEq;
		}
		else if (ch == '?')
		{
			if (!m_trie.isLeaf())
			{
				// Unknown command name
				m_state = &Server::stateError;
			}
			else
			{
				uint16_t cmd_index = m_trie.getCommandIndex();
				const detail::ExtCmdDef& cmd_def = Settings::ExtendedCommands::m_ext_cmd_defs[cmd_index];
				if (cmd_def.getReadMethod() == nullptr)
				{
					// This command can not be executed in read mode
					m_state = &Server::stateError;
				}
				else if (!addExtCmd(cmd_index, CMD_TYPE::READ))
				{
					// Buffer overflow
					m_state = &Server::stateError;
				}
				else
				{
					m_state = &Server::stateExtendedReadTest;
				}
			}
		}
		else if ((ch == ';') || (ch == getCommunicationParameters().getCmdLineTerminationChar()))
		{
			if (!m_trie.isLeaf())
			{
				// Unknown command name
				m_state = &Server::stateError;
			}
			else
			{
				uint16_t cmd_index = m_trie.getCommandIndex();
				const detail::ExtCmdDef& cmd_def = Settings::ExtendedCommands::m_ext_cmd_defs[cmd_index];
				if (cmd_def.getWriteMethod() == nullptr)
				{
					// This command can not be executed in write mode
					m_state = &Server::stateError;
					stateError(ch);
				}
				else
				{
					m_param_index = 0;
					if (!addExtCmd(cmd_index, CMD_TYPE::WRITE) || !finalizeExtCmd(cmd_def))
					{
						// Either the buffer oveflow
						// or the command has mandatory parameters
						m_state = &Server::stateError;
					}
					else
					{
						if (ch == getCommunicationParameters().getCmdLineTerminationChar())
						{
							startCmdExec();
						}
						else
						{
							m_trie.reset();
							m_state = &Server::stateBody;
						}
					}
				}
			}
		}
		else
		{
			// Parsing the command name
			if (!m_trie.feed(ch))
			{
				// Unknown command name
				m_state = &Server::stateError;
			}
		}
	}

	void stateExtendedEq(char ch, bool /*abortable*/)
	{
		if (ch == ' ')
		{
			return;
		}
		if (!m_trie.isLeaf())
		{
			// Unknown command name
			m_state = &Server::stateError;
			return;
		}

		uint16_t cmd_index = m_trie.getCommandIndex();

		if (ch == '?')
		{
			if (!addExtCmd(cmd_index, CMD_TYPE::TEST))
			{
				// Buffer overflow
				m_state = &Server::stateError;
			}
			else
			{
				m_state = &Server::stateExtendedReadTest;
			}
		}
		else
		{
			// The first symbol of write command parameters
			const detail::ExtCmdDef& cmd_def = Settings::ExtendedCommands::m_ext_cmd_defs[cmd_index];
			if (cmd_def.getWriteMethod() == nullptr)
			{
				// This command can not be executed in write mode
				m_state = &Server::stateError;
			}
			else
			{
				if (cmd_def.getParameters() == nullptr)
				{
					// This command does not accept parameters
					m_state = &Server::stateError;
				}
				else
				{
					if (!addExtCmd(cmd_index, CMD_TYPE::WRITE))
					{
						// Buffer overflow
						m_state = &Server::stateError;
					}
					else
					{
						m_param_index = 0;
						setupParameterParser();
						(this->*m_state)(ch, false);
					}
				}
			}
		}
	}

	void stateExtendedReadTest(char ch, bool /*abortable*/)
	{
		if (ch == ' ')
		{
			return;
		}
		if (ch == getCommunicationParameters().getCmdLineTerminationChar())
		{
			finalizeBasicCmd();
			startCmdExec();
		}
		else if (ch == ';')
		{
			finalizeBasicCmd();
			m_trie.reset();
			m_state = &Server::stateBody;
		}
		else
		{
			// Unexpected symbol
			m_state = &Server::stateError;
		}
	}

	bool processDefaultParameter(char ch, bool (Server::*add_default)(const detail::ExtCmdDef& cmd_def))
	{
		if (ch == ' ')
		{
			return true;
		}
		uint16_t cmd_index = m_trie.getCommandIndex();
		const detail::ExtCmdDef& cmd_def = Settings::ExtendedCommands::m_ext_cmd_defs[cmd_index];
		if (ch == ',')
		{
			if (cmd_def.getParameters()->count == (m_param_index + 1))
			{
				// Comma is not supported instead of the last parameter
				m_state = &Server::stateError;
			}
			else if (!(this->*add_default)(cmd_def))
			{
				// The parameter is not optional
				m_state = &Server::stateError;
			}
			else
			{
				setupParameterParser();
			}
			return true;
		}
		else if (ch == ';')
		{
			if (!finalizeExtCmd(cmd_def))
			{
				m_state = &Server::stateError;
			}
			else
			{
				m_trie.reset();
				m_state = &Server::stateBody;
			}
			return true;
		}
		else if (ch == getCommunicationParameters().getCmdLineTerminationChar())
		{
			if (!finalizeExtCmd(cmd_def))
			{
				stateError(ch);
			}
			else
			{
				startCmdExec();
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	void stateExtendedParamNumDecStart(char ch, bool /*abortable*/)
	{
		if (processDefaultParameter(ch, &Server::addDefaultNumericParameter))
		{
			return;
		}
		m_param_value_num = 0;
		m_state = &Server::stateExtendedParamNumDec;
		stateExtendedParamNumDec(ch);
	}

	void stateExtendedParamNumHexStart(char ch, bool /*abortable*/)
	{
		if (processDefaultParameter(ch, &Server::addDefaultNumericParameter))
		{
			return;
		}
		m_param_value_num = 0;
		m_state = &Server::stateExtendedParamNumHex;
		stateExtendedParamNumHex(ch);
	}

	void stateExtendedParamNumBinStart(char ch, bool /*abortable*/)
	{
		if (processDefaultParameter(ch, &Server::addDefaultNumericParameter))
		{
			return;
		}
		m_param_value_num = 0;
		m_state = &Server::stateExtendedParamNumBin;
		stateExtendedParamNumBin(ch);
	}

	void stateExtendedParamNumDec(char ch, bool abortable = false)
	{
		(void)abortable;

		if (processNumericParameterEnd_(ch))
		{
			return;
		}

		if (!processDecNumericChar(ch))
		{
			// Invalid character or overflow
			m_state = &Server::stateError;
		}
	}

	void stateExtendedParamNumHex(char ch, bool abortable = false)
	{
		(void)abortable;

		if (processNumericParameterEnd_(ch))
		{
			return;
		}

		if (m_param_value_num & 0xF0000000)
		{
			// Overflow
			m_state = &Server::stateError;
		}
		else
		{
			int_fast8_t hex = atcmd::detail::Characters::getHex(ch);
			if (hex == -1)
			{
				// Invalid character
				m_state = &Server::stateError;
			}
			else
			{
				m_param_value_num <<= 4;
				m_param_value_num += hex;
			}
		}
	}

	void stateExtendedParamNumBin(char ch, bool abortable = false)
	{
		(void)abortable;

		if (processNumericParameterEnd_(ch))
		{
			return;
		}

		if (m_param_value_num & 0x80000000)
		{
			// Overflow
			m_state = &Server::stateError;
		}
		else
		{
			if ((ch != '0') && (ch != '1'))
			{
				// Invalid character
				m_state = &Server::stateError;
			}
			else
			{
				m_param_value_num <<= 1;
				m_param_value_num |= (ch == '1');
			}
		}
	}

	void stateExtendedParamStringStart(char ch, bool /*abortable*/)
	{
		if (processDefaultParameter(ch, &Server::addDefaultStringParameter))
		{
			return;
		}

		if (ch != '"')
		{
			// Unknown symbol
			m_state = &Server::stateError;
		}
		else
		{
			const detail::ExtCmdDef& cmd_def = getCmdDef();
			const detail::ExtCmdParamDef& param_def = cmd_def.getParameters()->parameters[m_param_index];
			m_param_string_size = param_def.string_max_len;
			m_state = &Server::stateExtendedParamString;
		}
	}

	void stateExtendedParamHexStringStart(char ch, bool /*abortable*/)
	{
		if (processDefaultParameter(ch, &Server::addDefaultHexStringParameter))
		{
			return;
		}

		if (ch != '"')
		{
			// Unknown symbol
			m_state = &Server::stateError;
		}
		else
		{
			const detail::ExtCmdDef& cmd_def = getCmdDef();
			const detail::ExtCmdParamDef& param_def = cmd_def.getParameters()->parameters[m_param_index];
			m_param_hex_string.size = param_def.hexstring_max_size;
			m_param_hex_string.second = false;
			m_state = &Server::stateExtendedParamHexString;
		}
	}

	void stateExtendedParamString(char ch, bool /*abortable*/)
	{
		if (ch == '"')
		{
			// The end of string
			finalizeString(m_param_string_size);
			m_state = &Server::stateExtendedParamStringEnd;
		}
		else if (m_param_string_size == 1)
		{
			// String size exceeded
			m_state = &Server::stateError;
		}
		else
		{
			addChar(ch);
			m_param_string_size--;
		}
	}

	void stateExtendedParamHexString(char ch, bool /*abortable*/)
	{
		if ((ch == ' ') || (ch == '-'))
		{
			// Space and '-' may be used for formatting
			return;
		}
		if (ch == '"')
		{
			// The end of string
			if (m_param_hex_string.second)
			{
				// Uneven number of bytes
				m_state = &Server::stateError;
			}
			else
			{
				const detail::ExtCmdDef& cmd_def = getCmdDef();
				const detail::ExtCmdParamDef& param_def = cmd_def.getParameters()->parameters[m_param_index];
				finalizeHexString(m_param_hex_string.size, param_def.hexstring_max_size);
				m_state = &Server::stateExtendedParamStringEnd;
			}
			return;
		}

		int_fast8_t hex = atcmd::detail::Characters::getHex(ch);
		if (hex < 0)
		{
			// Invalid character
			m_state = &Server::stateError;
			stateError(ch);
			return;
		}

		if (!m_param_hex_string.second)
		{
			if (m_param_hex_string.size == 0)
			{
				// Size exceeded
				m_state = &Server::stateError;
				return;
			}
			m_param_hex_string.byte = static_cast<uint8_t>(hex) << 4;
		}
		else
		{
			m_param_hex_string.byte |= static_cast<uint8_t>(hex);
			addByte(m_param_hex_string.byte);
			m_param_hex_string.size--;
		}
		m_param_hex_string.second = !m_param_hex_string.second;
	}

	void stateExtendedParamStringEnd(char ch, bool /*abortable*/)
	{
		const detail::ExtCmdDef& cmd_def = getCmdDef();
		if (ch == ',')
		{
			// We`ve done with this parameter
			if (cmd_def.getParameters()->count == m_param_index )
			{
				// Comma is not supported after the last parameter
				m_state = &Server::stateError;
			}
			else
			{
				setupParameterParser();
			}
		}
		else if (ch == ';')
		{
			if (!finalizeExtCmd(cmd_def))
			{
				m_state = &Server::stateError;
			}
			else
			{
				m_trie.reset();
				m_state = &Server::stateBody;
			}
		}
		else if (ch == getCommunicationParameters().getCmdLineTerminationChar())
		{
			if (!finalizeExtCmd(cmd_def))
			{
				stateError(ch);
			}
			else
			{
				startCmdExec();
			}
		}
		else
		{
			// Unexpected character
			m_state = &Server::stateError;
		}
	}

	void stateError(char ch, bool abortable = false)
	{
		(void)abortable;
		if (ch == getCommunicationParameters().getCmdLineTerminationChar())
		{
			startCmdExec(true);
		}
	}

	void stateExecuting(char /*ch*/, bool abortable)
	{
		if (!abortable || !abortCmdExec())
		{
			return;
		}
		m_state = &Server::stateA;
	}

	const detail::ExtCmdDef& getCmdDef() const
	{
		return Settings::ExtendedCommands::m_ext_cmd_defs[m_trie.getCommandIndex()];
	}

	// Write parameter parsing
	void setupParameterParser()
	{
		const detail::ExtCmdDef& cmd_def = getCmdDef();
		if (m_param_index < cmd_def.getParameters()->count)
		{
			const detail::ExtCmdParamDef& param_def = cmd_def.getParameters()->parameters[m_param_index];
			if (checkParameterBufferOvf(param_def))
			{
				// Buffer overflow
				m_state = &Server::stateError;
				return;
			}

			switch (param_def.param_type) {
			case detail::ExtCmdParamDef::TYPE::NUM_DEC:
				m_param_value_num = 0;
				m_state = &Server::stateExtendedParamNumDecStart;
				break;
			case detail::ExtCmdParamDef::TYPE::NUM_HEX:
				m_state = &Server::stateExtendedParamNumHexStart;
				break;
			case detail::ExtCmdParamDef::TYPE::NUM_BIN:
				m_state = &Server::stateExtendedParamNumBinStart;
				break;
			case detail::ExtCmdParamDef::TYPE::STR:
				m_state = &Server::stateExtendedParamStringStart;
				break;
			case detail::ExtCmdParamDef::TYPE::STR_HEX:
				m_state = &Server::stateExtendedParamHexStringStart;
				break;
			default:
				break;
			}
		}
		else
		{
			// The number of parameters has been exceeded
			m_state = &Server::stateError;
		}
	}

	bool processNumericParameterEnd_(char ch)
	{
		const detail::ExtCmdDef& cmd_def = getCmdDef();
		bool r = true;

		if (ch == ',')
		{
			// We`ve done with this parameter
			if (cmd_def.getParameters()->count == (m_param_index + 1))
			{
				// Comma is not supported after the last parameter
				m_state = &Server::stateError;
			}
			else
			{
				if (!addNumericParameter(cmd_def))
				{
					m_state = &Server::stateError;
				}
				else
				{
					setupParameterParser();
				}
			}
		}
		else if (ch == ';')
		{
			if (!addNumericParameter(cmd_def) || !finalizeExtCmd(cmd_def))
			{
				m_state = &Server::stateError;
			}
			else
			{
				m_trie.reset();
				m_state = &Server::stateBody;
			}
		}
		else if (ch == getCommunicationParameters().getCmdLineTerminationChar())
		{
			if (!addNumericParameter(cmd_def) || !finalizeExtCmd(cmd_def))
			{
				stateError(ch);
			}
			else
			{
				startCmdExec();
			}
		}
		else
		{
			r = false;
		}
		return r;
	}

	bool processDecNumericChar(char ch)
	{
		int_fast8_t dec = atcmd::detail::Characters::getNumeric(ch);
		if (dec == -1)
		{
			// Invalid character
			return false;
		}
		if (m_param_value_num >= 0x19999999)
		{
			if ((m_param_value_num >= 0x1999999A) || (dec > 5))
			{
				// Overflow
				return false;
			};
		}
		m_param_value_num *= 10;
		m_param_value_num += dec;
		return true;
	}

	void startCmdExec(bool error = false)
	{
		m_state = &Server::stateExecuting;
		Base::startCmdExec(error);
		continueCmdExec();
	}

	void continueCmdExec()
	{
		if (Base::continueCmdExec())
		{
			m_state = &Server::stateA;
		}
	}

	void continueCmdExec(uint16_t cmd_id)
	{
		if (m_state != &Server::stateExecuting)
		{
			return;
		}
		if (Base::continueCmdExec(cmd_id))
		{
			m_state = &Server::stateA;
		}
	}

	State m_state;
};

} /* namespace atcmdlib::server */

#endif // ATCMD_SERVER_H
