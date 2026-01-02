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

#ifndef SERVER_CMDLINE_H
#define SERVER_CMDLINE_H

#include <atcmd/server/server_base.h>
#include <atcmd/detail/trie.h>
#include <atcmd/detail/basiccmddef.h>
#include <atcmd/detail/extcmddef.h>

namespace atcmd::server {

template<class... Cmds>
struct AmpersandCommandList;

template<>
struct AmpersandCommandList<>
{
	static constexpr inline std::size_t size = 0;
};

template<concepts::AmpersandCommand... Cmds>
struct AmpersandCommandList<Cmds...>
{
	static constexpr inline std::size_t size = sizeof...(Cmds);
	static constexpr inline detail::BasicCmdDef m_cmd_defs[] = {detail::BasicCmdDef::build<Cmds>()...};

	static int_fast8_t getCmdIndex(char name)
	{
		uint_fast8_t l = 0;
		uint_fast8_t r = size;
		while (l != r)
		{
			uint_fast8_t mid = (l + r) / 2;
			const detail::BasicCmdDef& el = m_cmd_defs[mid];
			if (name == el.name)
			{
				return mid;
			}
			else if (name < el.name)
			{
				r = mid;
			}
			else
			{
				l = mid + 1;
			}
		}
		return -1;
	}
};

template<concepts::BasicCommand... Cmds>
struct BasicCommandList : public AmpersandCommandList<Cmds...>
{};

template<concepts::ExtendedCommand... Cmds>
struct ExtendedCommandList
{
	static constexpr inline std::size_t size = sizeof...(Cmds);
	static constexpr inline detail::ExtCmdDef m_ext_cmd_defs[] = {detail::ExtCmdDef::build<Cmds>()...};
	using Trie = atcmd::detail::Trie<Cmds::Definition::name...>;
};

namespace concepts {

template<class T>
concept ServerSettings =
	requires
	{
		typename T::BasicCommands;
		typename T::AmpersandCommands;
		typename T::ExtendedCommands;
		{ T::max_commands_per_line } -> std::convertible_to<std::size_t>;
	}
	&& (T::max_commands_per_line > 0);

} /* namespace concepts */

namespace detail {

template<atcmd::server::concepts::ServerSettings Settings>
struct ServerCmdline : public detail::Server
{
protected:
	ServerCmdline(PrintCharCallback print_char_callback, void* user_handle = nullptr) :
		detail::Server(print_char_callback, user_handle)
	{}

	std::size_t getCmdlineBufSz()
	{
		return sizeof(m_cmdline) - m_cmdline_parse_index;
	}

	bool checkParameterBufferOvf(const detail::ExtCmdParamDef& param_def)
	{
		std::size_t available = getCmdlineBufSz();
		bool ovf;
		switch (param_def.param_type) {
		case detail::ExtCmdParamDef::TYPE::NUM_DEC:
		case detail::ExtCmdParamDef::TYPE::NUM_HEX:
		case detail::ExtCmdParamDef::TYPE::NUM_BIN:
			ovf = available < sizeof(uint32_t);
			break;
		case detail::ExtCmdParamDef::TYPE::STR:
			ovf = available < param_def.string_max_len;
			break;
		case detail::ExtCmdParamDef::TYPE::STR_HEX:
			ovf = available < (param_def.hexstring_max_size + sizeof(uint16_t));
			break;
		}
		return ovf;
	}

	void resetCmdline()
	{
		m_cmdline_parse_ok_index = 0;
		m_cmdline_parse_index = 0;
	}

	bool addBasicCmd(uint8_t index)
	{
		return addCmd(getBasicCmdOffset() + index);
	}

	bool addExtCmd(uint16_t cmd_index, CMD_TYPE cmd_type)
	{
		uint16_t vl = (cmd_index << 2) | static_cast<uint16_t>(cmd_type);
		return addCmd(vl);
	}

	bool addByteParameter()
	{
		if (getCmdlineBufSz() == 0)
		{
			// Buffer overflow
			return false;
		}
		m_cmdline[m_cmdline_parse_index++] = m_param_value_num & 0xFF;
		return true;
	}

	bool addNumericParameter(const detail::ExtCmdDef& cmd_def)
	{
		const detail::ExtCmdParamDef& param = cmd_def.getParameters()->parameters[m_param_index];
		return
				detail::BasicCmdDef::validateNumericRanges(*param.numeric_ranges, m_param_value_num) &&
				addNumericParameter_();
	}

	bool addNumericParameter(const detail::BasicCmdDef& cmd_def)
	{
		return
				detail::BasicCmdDef::validateNumericRanges(*cmd_def.numeric_ranges, m_param_value_num) &&
				addNumericParameter_();
	}

	void addChar(char ch)
	{
		m_cmdline[m_cmdline_parse_index++] = ch;
	}

	void addByte(uint8_t byte)
	{
		m_cmdline[m_cmdline_parse_index++] = byte;
	}

	bool addDefaultNumericParameter(const detail::ExtCmdDef& cmd_def)
	{
		const auto& p = cmd_def.getParameters()->parameters[m_param_index];
		if (!p.is_optional)
		{
			return false;
		}
		addDefaultNumericParameter_(p);
		return true;
	}

	bool addDefaultStringParameter(const detail::ExtCmdDef& cmd_def)
	{
		const auto& p = cmd_def.getParameters()->parameters[m_param_index];
		if (!p.is_optional)
		{
			return false;
		}
		addDefaultStringParameter_(p);
		return true;
	}

	bool addDefaultHexStringParameter(const detail::ExtCmdDef& cmd_def)
	{
		const auto& p = cmd_def.getParameters()->parameters[m_param_index];
		if (!p.is_optional)
		{
			return false;
		}
		addDefaultHexStringParameter_(p);
		return true;
	}

	void finalizeString(uint16_t size_left)
	{
		m_cmdline[m_cmdline_parse_index++] = '\0';
		m_cmdline_parse_index += size_left - 1;
		m_param_index++;
	}

	void finalizeHexString(uint16_t size_left, uint16_t size_full)
	{
		uint16_t size = size_full - size_left;
		m_cmdline_parse_index += size_left;
		m_cmdline[m_cmdline_parse_index++] = size & 0xFF;
		m_cmdline[m_cmdline_parse_index++] = (size >> 8) & 0xFF;
		m_param_index++;
	}

	void finalizeBasicCmd()
	{
		m_cmdline_parse_ok_index = m_cmdline_parse_index;
	}

	bool finalizeExtCmd(const detail::ExtCmdDef& cmd_def)
	{
		while (m_param_index != cmd_def.getParameters()->count)
		{
			const auto& p = cmd_def.getParameters()->parameters[m_param_index];
			if (!p.is_optional || checkParameterBufferOvf(p))
			{
				return false;
			}
			switch (p.param_type) {
			case detail::ExtCmdParamDef::TYPE::NUM_DEC:
			case detail::ExtCmdParamDef::TYPE::NUM_HEX:
			case detail::ExtCmdParamDef::TYPE::NUM_BIN:
				addDefaultNumericParameter_(p);
				break;
			case detail::ExtCmdParamDef::TYPE::STR:
				addDefaultStringParameter_(p);
				break;
			case detail::ExtCmdParamDef::TYPE::STR_HEX:
				addDefaultHexStringParameter_(p);
				break;
			default:
				break;
			}
		}
		m_cmdline_parse_ok_index = m_cmdline_parse_index;
		return true;
	}

	void startCmdExec(bool error)
	{
		m_cmdline_exec_index = 0;
		m_last_result_code = RESULT_CODE::OK;
		m_error = error;
	}

	bool continueCmdExec()
	{
		while (m_cmdline_exec_index != m_cmdline_parse_ok_index)
		{
			// Extract cmd index
			uint16_t vl = m_cmdline[m_cmdline_exec_index] & 0x7F;
			uint_fast8_t vl_size;
			if (m_cmdline[m_cmdline_exec_index] & 0x80)
			{
				vl += m_cmdline[m_cmdline_exec_index + 1] << 7;
				vl_size = 2;
			}
			else
			{
				vl_size = 1;
			}

			if constexpr ((Settings::BasicCommands::size != 0) || (Settings::AmpersandCommands::size != 0))
			{
				if (vl >= getBasicCmdOffset())
				{
					uint16_t cmd_index = vl - getBasicCmdOffset();
					execBasicCmd(cmd_index, vl_size);
				}
				else
				{
					uint16_t cmd_index = vl >> 2;
					CMD_TYPE cmd_type = static_cast<CMD_TYPE>(vl & 0x03);
					execExtendedCmd(cmd_index, cmd_type, vl_size);
				}
			}
			else
			{
				uint16_t cmd_index = vl >> 2;
				CMD_TYPE cmd_type = static_cast<CMD_TYPE>(vl & 0x03);
				execExtendedCmd(cmd_index, cmd_type, vl_size);
			}

			if (m_last_result_code == RESULT_CODE::ERROR)
			{
				return false;
			}
			if (m_last_result_code == RESULT_CODE::ASYNC)
			{
				return false;
			}
		}
		if (m_error)
		{
			m_last_result_code = RESULT_CODE::ERROR;
		}
		printResultCode(m_last_result_code);
		return true;
	}

	// Parameter parsing
	union
	{
		uint8_t m_basic_cmd_index;
		uint8_t m_param_index;
	};

	struct HexString
	{
		uint16_t size;
		uint8_t byte;
		bool second;
	};

	union
	{
		uint32_t m_param_value_num;
		uint32_t m_param_string_size;
		HexString m_param_hex_string;
	};

private:
	static consteval uint16_t getBasicCmdOffset()
	{
		return Settings::ExtendedCommands::size << 2;
	}

	bool addCmd(uint16_t vl)
	{
		uint_fast8_t sz = 1 + (vl > 0x7F);
		if (getCmdlineBufSz() < sz)
		{
			return false;
		}
		m_cmdline[m_cmdline_parse_index] = vl & 0x7F;
		if (sz == 2)
		{
			m_cmdline[m_cmdline_parse_index++] |= 0x80;
			vl >>= 7;
			m_cmdline[m_cmdline_parse_index++] = vl & 0xFF;
		}
		else
		{
			m_cmdline_parse_index++;
		}
		return true;
	}

	bool addNumericParameter_()
	{
		for (uint_fast8_t i = 0; i < sizeof(m_param_value_num); i++)
		{
			m_cmdline[m_cmdline_parse_index++] = m_param_value_num & 0xFF;
			m_param_value_num >>= 8;
		}
		m_param_index++;
		return true;
	}

	void addDefaultNumericParameter_(const detail::ExtCmdParamDef& param)
	{
		m_param_value_num = param.default_number;
		addNumericParameter_();
	}

	void addDefaultStringParameter_(const detail::ExtCmdParamDef& param)
	{
		uint16_t i = param.string_max_len;
		for (const char* c = param.default_string; *c != '\0'; c++)
		{
			m_cmdline[m_cmdline_parse_index++] = *c;
			i--;
		}
		finalizeString(i);
	}

	void addDefaultHexStringParameter_(const detail::ExtCmdParamDef& param)
	{
		m_cmdline[m_cmdline_parse_index + param.default_hex_string->size]
				= param.default_hex_string->size & 0xFF;
		m_cmdline[m_cmdline_parse_index + param.default_hex_string->size + 1]
				= (param.default_hex_string->size >> 8) & 0xFF;
		for (std::size_t i = 0; i < param.default_hex_string->size; i++)
		{
			m_cmdline[m_cmdline_parse_index++] = param.default_hex_string->data[i];
		}
		m_cmdline_parse_index += sizeof(uint16_t);
	}

	void execBasicCmd(uint16_t cmd_index, uint_fast8_t vl_size)
	{
		if (cmd_index == 0)
		{
			// S parameter
			uint8_t param_index = m_cmdline[m_cmdline_exec_index + vl_size];
			if (param_index & 0x80)
			{
				// Write request
				param_index &= 0x7F;
				char ch = m_cmdline[m_cmdline_exec_index + vl_size + 1];
				switch (param_index) {
				case 3:
					getCommunicationParameters().setCmdLineTerminationChar(ch);
					break;
				case 4:
					getCommunicationParameters().setResponseFormattingChar(ch);
				default:
					break;
				}
				m_cmdline_exec_index += vl_size + 2;
			}
			else
			{
				// Read request
				uint8_t r;
				switch (param_index) {
				case 3:
					r = getCommunicationParameters().getCmdLineTerminationChar();
					break;
				case 4:
					r = getCommunicationParameters().getResponseFormattingChar();
				default:
					break;
				}

				printInformationTextHeader();
				for (uint8_t div = 100; div != 0; div /= 10)
				{
					printChar('0' + (r / div));
					r %= div;
				}
				printInformationTextTrailer();

				m_cmdline_exec_index += vl_size + 1;
			}
			m_last_result_code = RESULT_CODE::OK;
			return;
		}

		cmd_index--;

		const BasicCmdDef* cmd_def;
		if (cmd_index < Settings::BasicCommands::size)
		{
			if constexpr (Settings::BasicCommands::size != 0)
			{
				cmd_def = &Settings::BasicCommands::m_cmd_defs[cmd_index];
			}
		}
		else
		{
			if constexpr (Settings::AmpersandCommands::size != 0)
			{
				cmd_def = &Settings::AmpersandCommands::m_cmd_defs[cmd_index - Settings::BasicCommands::size];
			}
		}

		std::size_t next_exec_index = m_cmdline_exec_index + vl_size;
		const uint8_t* param_start;
		if (cmd_def->numeric_ranges != nullptr)
		{
			next_exec_index += sizeof(uint32_t);
			param_start = &m_cmdline[m_cmdline_exec_index + vl_size];
		}
		else
		{
			param_start = nullptr;
		}
		bool is_last = m_cmdline_parse_index == next_exec_index;

		m_last_result_code = cmd_def->exec_method(getBasicHandle(param_start, is_last));

		if (m_last_result_code != RESULT_CODE::ASYNC)
		{
			// Go to the next command
			m_cmdline_exec_index = next_exec_index;
		}
	}

	void execExtendedCmd(uint16_t cmd_index, CMD_TYPE cmd_type, uint_fast8_t vl_size)
	{
		const detail::ExtCmdDef& cmd_def = Settings::ExtendedCommands::m_ext_cmd_defs[cmd_index];

		std::size_t next_exec_index = m_cmdline_exec_index + vl_size;
		if ((cmd_type == CMD_TYPE::WRITE) && (cmd_def.getParameters() != nullptr))
		{
			for (std::size_t j = 0; j < cmd_def.getParameters()->count; j++)
			{
				switch (cmd_def.getParameters()->parameters[j].param_type) {
				case detail::ExtCmdParamDef::TYPE::NUM_DEC:
				case detail::ExtCmdParamDef::TYPE::NUM_HEX:
				case detail::ExtCmdParamDef::TYPE::NUM_BIN:
					next_exec_index += sizeof(uint32_t);
					break;
				case detail::ExtCmdParamDef::TYPE::STR:
					next_exec_index += cmd_def.getParameters()->parameters[j].string_max_len;
					break;
				case detail::ExtCmdParamDef::TYPE::STR_HEX:
					next_exec_index += cmd_def.getParameters()->parameters[j].hexstring_max_size + sizeof(uint16_t);
					break;
				default:
					break;
				}
			}
		}
		bool is_last = m_cmdline_parse_index == next_exec_index;
		switch (cmd_type) {
		case CMD_TYPE::READ:
			m_last_result_code = cmd_def.getReadMethod()(getReadHandle(is_last));
			break;
		case CMD_TYPE::WRITE:
			m_last_result_code = cmd_def.getWriteMethod()(getWriteHandle(&m_cmdline[m_cmdline_exec_index + vl_size], is_last));
			break;
		case CMD_TYPE::TEST:
			for (detail::ExtendedCommandBase::TestMethod method = cmd_def.getTestMethod(); method != nullptr;)
			{
				const char* name = nullptr;
				if (method != nullptr)
				{
					name = method(getTestHandle(is_last));
				}
				if (name != nullptr)
				{
					printCmdParameterRanges(cmd_def, name);
				}
				break;
			}
			m_last_result_code = RESULT_CODE::OK;
			break;
		}

		if (m_last_result_code != RESULT_CODE::ASYNC)
		{
			// Go to the next command
			m_cmdline_exec_index = next_exec_index;
		}
	}

	void printCmdParameterRanges(const detail::ExtCmdDef& cmd_def, const char* name)
	{
		if (cmd_def.getParameters() == nullptr)
		{
			return;
		}

		printInformationTextHeader();
		printExtendedInformationTextHeader(name);

		for (std::size_t i = 0;;)
		{
			printChar('(');

			const auto& parameter = cmd_def.getParameters()->parameters[i];
			uint_fast8_t base;
			switch (parameter.param_type) {
			case detail::ExtCmdParamDef::TYPE::NUM_DEC:
				base = 10;
				break;
			case detail::ExtCmdParamDef::TYPE::NUM_HEX:
				base = 16;
				break;
			case detail::ExtCmdParamDef::TYPE::NUM_BIN:
				base = 2;
				break;
			default:
				break;
			}

			switch (parameter.param_type) {
			case detail::ExtCmdParamDef::TYPE::NUM_DEC:
			case detail::ExtCmdParamDef::TYPE::NUM_HEX:
			case detail::ExtCmdParamDef::TYPE::NUM_BIN:
				for (std::size_t j = 0;;)
				{
					const auto& range = parameter.numeric_ranges->ranges[j];
					printNumber(range.m_min, base);
					if (range.m_min != range.m_max)
					{
						printChar('-');
						printNumber(range.m_max, base);
					}
					if (++j == parameter.numeric_ranges->count)
					{
						break;
					}
					else
					{
						printChar(',');
					}
				}
				break;
			case detail::ExtCmdParamDef::TYPE::STR:
				printText("s:");
				printNumber(parameter.string_max_len - 1, 10);
				break;
			case detail::ExtCmdParamDef::TYPE::STR_HEX:
				printText("hs:");
				printNumber(parameter.hexstring_max_size, 10);
				break;
			default:
				break;
			}

			printChar(')');

			if (++i == cmd_def.getParameters()->count)
			{
				break;
			}
			else
			{
				printChar(',');
			}
		}

		printInformationTextTrailer();
	}

	static consteval std::size_t calcCmdlineSize()
	{
		// No more than 6 bytes needed to encode a basic command
		// 2 bytes for index
		// 4 bytes for numeric parameter
		std::size_t r = 6;

		if constexpr (Settings::ExtendedCommands::size != 0)
		{
			for (const detail::ExtCmdDef& def : Settings::ExtendedCommands::m_ext_cmd_defs)
			{
				if (!def.getFlags().writable)
				{
					continue;
				}
				const detail::ExtCmdDef::Parameters* parameters = def.getParameters();
				if (parameters->count != 0)
				{
					std::size_t n = 2;
					for (uint_fast8_t i = 0; i < parameters->count; i++)
					{
						const ExtCmdParamDef* p = &parameters->parameters[i];
						switch (p->param_type) {
						case ExtCmdParamDef::TYPE::NUM_DEC:
						case ExtCmdParamDef::TYPE::NUM_HEX:
						case ExtCmdParamDef::TYPE::NUM_BIN:
							n += sizeof(uint32_t);
							break;
						case ExtCmdParamDef::TYPE::STR:
							n += p->string_max_len;
							break;
						case ExtCmdParamDef::TYPE::STR_HEX:
							n += p->hexstring_max_size + sizeof(uint16_t);
							break;
						default:
							break;
						}
					}
					if (n > r)
					{
						r = n;
					}
				}
			}
		}

		return r * Settings::max_commands_per_line;
	}

	uint8_t m_cmdline[calcCmdlineSize()];
	std::size_t m_cmdline_parse_ok_index;
	std::size_t m_cmdline_parse_index;
	std::size_t m_cmdline_exec_index;
	RESULT_CODE m_last_result_code;
	bool m_error;
};

} /* namespace detail */
} /* namespace atcmd::server */

#endif // SERVER_CMDLINE_H
