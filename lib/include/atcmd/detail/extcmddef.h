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

#ifndef ATCMD_EXTCMDDEF_H
#define ATCMD_EXTCMDDEF_H

#include <array>

#include <atcmd/detail/cmdparamdef.h>
#include <atcmd/server/extendedcommand.h>

namespace atcmd::server::detail {

struct ExtCmdParamDef : public CmdParamDef
{
	enum class TYPE
	{
		NUM_DEC,
		NUM_HEX,
		NUM_BIN,
		STR,
		STR_HEX
	};

	TYPE param_type : 4;
	bool is_optional : 1;

	union
	{
		const Ranges* numeric_ranges;
		uint16_t string_max_len;
		uint16_t hexstring_max_size;
	};

	struct HexString
	{
		const uint8_t* data;
		uint16_t size;
	};

	union
	{
		uint32_t default_number;
		const char* default_string;
		const HexString* default_hex_string;
	};

	template<class Parameter>
	static consteval ExtCmdParamDef build()
	{
		ExtCmdParamDef r;
		if constexpr (atcmd::server::concepts::NumericParameter<Parameter>)
		{
			static constexpr Ranges ranges =
			{
				.count = std::size(Parameter::ranges),
				.ranges = Parameter::ranges
			};
			r.numeric_ranges = &ranges;
		}
		if constexpr (atcmd::server::concepts::DecimalNumericParameter<Parameter>)
		{
			r.param_type = TYPE::NUM_DEC;
		}
		else if constexpr (atcmd::server::concepts::HexadecimalNumericParameter<Parameter>)
		{
			r.param_type = TYPE::NUM_HEX;
		}
		else if constexpr (atcmd::server::concepts::BinaryNumericParameter<Parameter>)
		{
			r.param_type = TYPE::NUM_BIN;
		}
		else if constexpr (atcmd::server::concepts::StringParameter<Parameter>)
		{
			r.param_type = TYPE::STR;
			r.string_max_len = Parameter::max_length + 1; // +1 for \0 character
		}
		else if constexpr (atcmd::server::concepts::HexadecimalStringParameter<Parameter>)
		{
			r.param_type = TYPE::STR_HEX;
			r.hexstring_max_size = Parameter::max_size;
		}
		else
		{
			static_assert(false, "Unknown parameter type");
		}
		r.is_optional = Parameter::is_optional;
		if constexpr (Parameter::is_optional)
		{
			if constexpr (
						  atcmd::server::concepts::DecimalNumericParameter<Parameter> ||
						  atcmd::server::concepts::HexadecimalNumericParameter<Parameter> ||
						  atcmd::server::concepts::BinaryNumericParameter<Parameter>)
			{
				r.default_number = Parameter::default_value;
			}
			else if constexpr (atcmd::server::concepts::StringParameter<Parameter>)
			{
				r.default_string = Parameter::default_value;
			}
			else if constexpr (atcmd::server::concepts::HexadecimalStringParameter<Parameter>)
			{
				static constexpr HexString hex_string =
				{
					.data = Parameter::default_value,
					.size = std::size(Parameter::default_value)
				};
				r.default_hex_string = &hex_string;
			}
			else
			{
				static_assert(false, "We have a bug somewhere");
			}
		}
		return r;
	}
};

class ExtCmdDef
{
public:
	struct Flags
	{
		bool readable : 1;
		bool writable : 1;
		bool custom_testable : 1;
		bool abortable : 1;

		bool single_method : 1;
	};

	struct Parameters
	{
		uint8_t count;
		const ExtCmdParamDef* parameters;
	};

private:
	union Method
	{
		ExtendedCommandBase::WriteMethod write;
		ExtendedCommandBase::ReadMethod read;
		ExtendedCommandBase::TestMethod test;
		ExtendedCommandBase::AbortMethod abort;
	};

	union Methods
	{
		Method method;
		const Method* methods;
	};

	template<class... T>
	struct ParameterBuilderBase
	{
		static constexpr ExtCmdParamDef params[] = {ExtCmdParamDef::build<T>()...};
		static constexpr Parameters parameters =
		{
			.count = sizeof(params) / sizeof(params[0]),
			.parameters = params
		};
	};

	struct ParameterBuilderStub
	{
		static constexpr Parameters parameters =
		{
			.count = 0,
			.parameters = nullptr
		};
	};

	template<class T>
	struct ParameterBuilder;

	template<class... T>
	struct ParameterBuilder<atcmd::server::ExtendedCommand::ParameterList<T...>> :
		public std::conditional_t<(sizeof...(T) > 0), ParameterBuilderBase<T...>, ParameterBuilderStub>
	{};

	template<class AtCmd, Flags flags, std::size_t N>
	class MethodBuilder
	{
		static consteval std::array<Method, N> getMethods()
		{
			std::array<Method, N> r;
			std::size_t i = 0;
			if constexpr (flags.readable)
			{
				r[i++].read = AtCmd::Definition::onRead;
			}
			if constexpr (flags.writable)
			{
				r[i++].write = AtCmd::Definition::onWrite;
			}
			if constexpr (flags.custom_testable)
			{
				r[i++].test = AtCmd::Definition::onTest;
			}
			if constexpr (flags.abortable)
			{
				r[i++].abort = AtCmd::Definition::onAbort;
			}
			return r;
		}

	public:
		static constexpr auto methods = getMethods();
	};

public:
	template<class AtCmd>
	static consteval ExtCmdDef build()
	{
		static constexpr Flags flags =
		{
			.readable = atcmd::server::concepts::ExtendedReadCommand<AtCmd>,
			.writable = atcmd::server::concepts::ExtendedWriteCommand<AtCmd>,
			.custom_testable = atcmd::server::concepts::ExtendedTestCommand<AtCmd>,
			.abortable = atcmd::server::concepts::ExtendedAbortCommand<AtCmd>,
			.single_method = false
		};
		static constexpr uint8_t method_count =
				flags.readable + flags.writable + flags.custom_testable + flags.abortable;

		ExtCmdDef r = {};
		r.m_flags = flags;

		if constexpr (method_count == 1)
		{
			r.m_flags.single_method = true;
			if constexpr (flags.readable)
			{
				r.m_methods.method.read = AtCmd::Definition::onRead;
			}
			if constexpr (flags.writable)
			{
				r.m_methods.method.write = AtCmd::Definition::onWrite;
			}
			if constexpr (flags.custom_testable)
			{
				r.m_methods.method.test = AtCmd::Definition::onTest;
			}
			if constexpr (flags.abortable)
			{
				r.m_methods.method.abort = AtCmd::Definition::onAbort;
			}
		}
		else
		{
			r.m_methods.methods = MethodBuilder<AtCmd, flags, method_count>::methods.data();
		}

		using ParamBuider = ParameterBuilder<typename AtCmd::Definition::Parameters>;
		if constexpr (ParamBuider::parameters.count > 0)
		{
			r.m_parameters = &ParamBuider::parameters;
		}
		else
		{
			r.m_parameters = nullptr;
		}

		return r;
	}

	ExtendedCommandBase::ReadMethod getReadMethod() const;
	ExtendedCommandBase::WriteMethod getWriteMethod() const;
	ExtendedCommandBase::TestMethod getTestMethod() const;
	ExtendedCommandBase::AbortMethod getAbortMethod() const;

	constexpr const Parameters* getParameters() const
	{
		return m_parameters;
	}

	consteval Flags getFlags() const
	{
		return m_flags;
	}

private:
	Methods m_methods;
	const Parameters* m_parameters;
	Flags m_flags;
};

} /* namespace atcmd::server::detail */

#endif // ATCMD_EXTCMDDEF_H
