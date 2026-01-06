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

#ifndef ATCMD_EXTENDEDCOMMAND_H
#define ATCMD_EXTENDEDCOMMAND_H

#include <concepts>
#include <cstdint>
#include <string_view>
#include <span>

#include <atcmd/common.h>
#include <atcmd/server/command_base.h>
#include <atcmd/detail/characters.h>

namespace atcmd::server {

namespace detail {

struct ExtendedCommandBase1 : protected Command
{
	struct DecimalNumericParameter : public NumericParameter
	{};

	struct HexadecimalNumericParameter : public NumericParameter
	{};

	struct BinaryNumericParameter : public NumericParameter
	{};

	struct StringParameter : public Parameter
	{};

	struct HexadecimalStringParameter : public Parameter
	{};
};

} /* namespace detail */

namespace concepts {

template<class T>
concept ExtendedParameter =
	Parameter<T> &&
	requires
	{
		{ T::is_optional } -> std::same_as<const bool&>;
	} &&
	(T::is_optional == requires{T::default_value;});

template<class T>
concept ExtendedNumericParameter =
	ExtendedParameter<T> &&
	NumericParameter<T> &&
	(!T::is_optional || ((
		requires
		{
			{T::default_value} -> std::same_as<const uint32_t&>;
		}) &&
		detail::CmdParamDef::validateNumericRanges(
							 {
								 .count = std::size(T::ranges),
								 .ranges = T::ranges
							 },
							 T::default_value))
		);

template<class T>
concept DecimalNumericParameter =
	ExtendedNumericParameter<T> && std::derived_from<T, detail::ExtendedCommandBase1::DecimalNumericParameter>;

template<class T>
concept HexadecimalNumericParameter =
	ExtendedNumericParameter<T> && std::derived_from<T, detail::ExtendedCommandBase1::HexadecimalNumericParameter>;

template<class T>
concept BinaryNumericParameter =
	ExtendedNumericParameter<T> && std::derived_from<T, detail::ExtendedCommandBase1::BinaryNumericParameter>;

template<class T>
concept StringParameter =
	ExtendedParameter<T> &&
	std::derived_from<T, detail::ExtendedCommandBase1::StringParameter> &&
	requires
	{
		{ T::max_length } -> std::same_as<const uint16_t&>;
	} &&
	(T::max_length <= 0xFFFE) &&
	(!T::is_optional || (
		requires
		{
			{T::default_value} -> std::same_as<const char* const&>;
		} &&
		(std::string_view(T::default_value).size() <= T::max_length)
	));

template<class T>
concept HexadecimalStringParameter =
	ExtendedParameter<T> &&
	std::derived_from<T, detail::ExtendedCommandBase1::HexadecimalStringParameter> &&
	requires
	{
		{ T::max_size } -> std::same_as<const uint16_t&>;
	} &&
	(!T::is_optional || (
		requires
		{
			{T::default_value} -> std::same_as<const uint8_t (&)[std::extent_v<decltype(T::default_value)>]>;
		} &&
		(std::size(T::default_value) <= T::max_size)
	));

} /* namespace concepts */

namespace detail {

struct ExtendedCommandBase : public ExtendedCommandBase1
{
	template<atcmd::server::concepts::Parameter... Ts>
	class ParameterList : private ExtendedCommandBase1::ParameterList<Ts...>
	{
		using Base = ExtendedCommandBase1::ParameterList<Ts...>;
		using Base::m_params;

		template<atcmd::server::concepts::Parameter P, atcmd::server::concepts::Parameter... Ps>
		struct OffsetCalc : public Base::template OffsetCalc<P, Ps...>
		{};

		template<atcmd::server::concepts::Parameter P, atcmd::server::concepts::Parameter... Ps>
		struct OffsetCalc<P, P, Ps...>
		{
			static constexpr std::size_t offset = 0;
		};

		template<atcmd::server::concepts::Parameter P, atcmd::server::concepts::StringParameter S, atcmd::server::concepts::Parameter... Ps>
		struct OffsetCalc<P, S, Ps...>
		{
			static constexpr std::size_t offset = S::max_length + 1 + OffsetCalc<P, Ps...>::offset;
		};

		template<atcmd::server::concepts::Parameter P, atcmd::server::concepts::HexadecimalStringParameter H, atcmd::server::concepts::Parameter... Ps>
		struct OffsetCalc<P, H, Ps...>
		{
			static constexpr std::size_t offset = H::max_size + sizeof(uint16_t) + OffsetCalc<P, Ps...>::offset;
		};

	public:
		ParameterList(const ParamServerHandle& handle) :
			ExtendedCommandBase1::ParameterList<Ts...>(handle)
		{}

		template<atcmd::server::concepts::NumericParameter N>
		uint32_t getNumeric() const
		{
			return Base::getNumeric_(OffsetCalc<N, Ts...>::offset);
		}

		template<atcmd::server::concepts::StringParameter S>
		const char* getString() const
		{
			return reinterpret_cast<const char*>(&m_params[OffsetCalc<S, Ts...>::offset]);
		}

		template<atcmd::server::concepts::HexadecimalStringParameter H>
		std::span<const uint8_t> getHexString() const
		{
			static constexpr std::size_t offset = OffsetCalc<H, Ts...>::offset;
			static constexpr std::size_t sz_start = offset + H::max_size;
			uint16_t sz = m_params[sz_start] | (m_params[sz_start + 1] << 8);
			return std::span<const uint8_t>(&m_params[offset], sz);
		}
	};

	class TestServerHandle : public ServerHandle
	{
		friend class Server;

	protected:
		struct ExtendedInformationText : public InformationText
		{
			ExtendedInformationText(Server& server, bool is_result_code, const char* name, bool is_silent);
			~ExtendedInformationText();

		private:
			PrintCharCallback m_print_char_callback;
		};

		TestServerHandle(Server& server, bool is_last_command);
	};

	struct WriteServerHandle :
			public TestServerHandle,
			public ParamServerHandle
	{
		friend class Server;
		friend struct ExtendedCommandBase;

	private:
		WriteServerHandle(const uint8_t* param_start, Server& server, bool is_last_command);
	};

	class ReadServerHandle : public TestServerHandle
	{
		friend class Server;
		struct ParameterInformationTextSecond;

		template<class>
		struct ParameterInformationTextSecondTmpl;

		struct ParameterInformationText : public ExtendedInformationText
		{
			friend struct ParameterInformationTextSecond;

			ParameterInformationText(Server& server, bool is_result_code, const char* name, bool is_silent);

			ParameterInformationTextSecond printNumericParameter(uint32_t value, uint8_t base) &&;
			ParameterInformationTextSecond printNumericParameter(uint32_t value, uint8_t base) & = delete;

			ParameterInformationTextSecond printStringParameter(const char* s) &&;
			ParameterInformationTextSecond printStringParameter(const char* s) & = delete;

			ParameterInformationTextSecond printHexadecimalStringParameter(const uint8_t* data, uint16_t size) &&;
			ParameterInformationTextSecond printHexadecimalStringParameter(const uint8_t* data, uint16_t size) & = delete;

		protected:
			void printNumericParameter_(uint32_t value, uint8_t base);
			void printStringParameter_(const char* s);
			void printHexadecimalStringParameter_(const uint8_t* data, uint16_t size);
		};

		template<class>
		struct ParameterInformationTextTmpl;

		template<atcmd::server::concepts::DecimalNumericParameter T, class... Ts>
		struct ParameterInformationTextTmpl<ParameterList<T, Ts...>> : private ParameterInformationText
		{
			ParameterInformationTextTmpl(Server& server, bool is_result_code, const char* name, bool is_silent) :
				ParameterInformationText(server, is_result_code, name, is_silent)
			{}

			template<atcmd::server::concepts::DecimalNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base = 10) &&
			{
				static_assert(std::same_as<P, T>, "Wrong order of parameters");
				printNumericParameter_(value, base);
				return ParameterInformationTextSecondTmpl<ParameterList<Ts...>>(m_server);
			}

			template<atcmd::server::concepts::DecimalNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base) & = delete;
		};

		template<atcmd::server::concepts::HexadecimalNumericParameter T, class... Ts>
		struct ParameterInformationTextTmpl<ParameterList<T, Ts...>> : private ParameterInformationText
		{
			ParameterInformationTextTmpl(Server& server, bool is_result_code, const char* name, bool is_silent) :
				ParameterInformationText(server, is_result_code, name, is_silent)
			{}

			template<atcmd::server::concepts::HexadecimalNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base = 16) &&
			{
				static_assert(std::same_as<P, T>, "Wrong order of parameters");
				printNumericParameter_(value, base);
				// TODO probably a compiler bug
				return atcmd::server::detail::ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecondTmpl<atcmd::server::detail::ExtendedCommandBase::ParameterList<Ts...>>(m_server);
				//return ParameterInformationTextSecondTmpl<ParameterList<Ts...>>(m_server);
			}

			template<atcmd::server::concepts::HexadecimalNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base) & = delete;
		};

		template<atcmd::server::concepts::BinaryNumericParameter T, class... Ts>
		struct ParameterInformationTextTmpl<ParameterList<T, Ts...>> : private ParameterInformationText
		{
			ParameterInformationTextTmpl(Server& server, bool is_result_code, const char* name, bool is_silent) :
				ParameterInformationText(server, is_result_code, name, is_silent)
			{}

			template<atcmd::server::concepts::BinaryNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base = 2) &&
			{
				static_assert(std::same_as<P, T>, "Wrong order of parameters");
				printNumericParameter_(value, base);
				// TODO probably a compiler bug
				return atcmd::server::detail::ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecondTmpl<atcmd::server::detail::ExtendedCommandBase::ParameterList<Ts...>>(m_server);
				//return ParameterInformationTextSecondTmpl<ParameterList<Ts...>>(m_server);
			}

			template<atcmd::server::concepts::BinaryNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base) & = delete;
		};

		template<atcmd::server::concepts::StringParameter T, class... Ts>
		struct ParameterInformationTextTmpl<ParameterList<T, Ts...>> : private ParameterInformationText
		{
			ParameterInformationTextTmpl(Server& server, bool is_result_code, const char* name, bool is_silent) :
				ParameterInformationText(server, is_result_code, name, is_silent)
			{}

			template<atcmd::server::concepts::StringParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printStringParameter(const char* s) &&
			{
				static_assert(std::same_as<P, T>, "Wrong order of parameters");
				printStringParameter_(s);
				// TODO probably a compiler bug
				return atcmd::server::detail::ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecondTmpl<atcmd::server::detail::ExtendedCommandBase::ParameterList<Ts...>>(m_server);
				//return ParameterInformationTextSecondTmpl<ParameterList<Ts...>>(m_server);
			}

			template<atcmd::server::concepts::StringParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printStringParameter(const char* s) & = delete;
		};

		template<atcmd::server::concepts::HexadecimalStringParameter T, class... Ts>
		struct ParameterInformationTextTmpl<ParameterList<T, Ts...>> : private ParameterInformationText
		{
			ParameterInformationTextTmpl(Server& server, bool is_result_code, const char* name, bool is_silent) :
				ParameterInformationText(server, is_result_code, name, is_silent)
			{}

			template<atcmd::server::concepts::HexadecimalStringParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printHexadecimalStringParameter(const uint8_t* data, uint16_t size) &&
			{
				static_assert(std::same_as<P, T>, "Wrong order of parameters");
				printHexadecimalStringParameter_(data, size);
				// TODO probably a compiler bug
				return atcmd::server::detail::ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecondTmpl<atcmd::server::detail::ExtendedCommandBase::ParameterList<Ts...>>(m_server);
				//return ParameterInformationTextSecondTmpl<ParameterList<Ts...>>(m_server);
			}

			template<atcmd::server::concepts::HexadecimalStringParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printHexadecimalStringParameter(const uint8_t* data, uint16_t size) & = delete;
		};

		struct ParameterInformationTextSecond
		{
			explicit ParameterInformationTextSecond(Server& server);

			ParameterInformationTextSecond&& printNumericParameter(uint32_t value, uint8_t base) &&;
			ParameterInformationTextSecond&& printNumericParameter(uint32_t value, uint8_t base) & = delete;

			ParameterInformationTextSecond&& printStringParameter(const char* s) &&;
			ParameterInformationTextSecond&& printStringParameter(const char* s) & = delete;

			ParameterInformationTextSecond&& printHexadecimalStringParameter(const uint8_t* data, uint16_t size) &&;
			ParameterInformationTextSecond&& printHexadecimalStringParameter(const uint8_t* data, uint16_t size) & = delete;

		protected:
			void printNumericParameter_(uint32_t value, uint8_t base);
			void printStringParameter_(const char* s);
			void printHexadecimalStringParameter_(const uint8_t* data, uint16_t size);

			detail::Server& m_server;
		};

		template<template<class...> class Pl>
		struct ParameterInformationTextSecondTmpl<Pl<>>
		{
			explicit ParameterInformationTextSecondTmpl(Server&) {};
		};

		template<atcmd::server::concepts::DecimalNumericParameter T, class... Ts>
		struct ParameterInformationTextSecondTmpl<ParameterList<T, Ts...>> : private ParameterInformationTextSecond
		{
			explicit ParameterInformationTextSecondTmpl(Server& server) :
				ParameterInformationTextSecond(server)
			{}

			template<atcmd::server::concepts::DecimalNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base = 10) &&
			{
				static_assert(std::same_as<P, T>, "Wrong order of parameters");
				printNumericParameter_(value, base);
				return ParameterInformationTextSecondTmpl<ParameterList<Ts...>>(m_server);
			}

			template<atcmd::server::concepts::DecimalNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base) & = delete;
		};

		template<atcmd::server::concepts::HexadecimalNumericParameter T, class... Ts>
		struct ParameterInformationTextSecondTmpl<ParameterList<T, Ts...>> : private ParameterInformationTextSecond
		{
			explicit ParameterInformationTextSecondTmpl(Server& server) :
				ParameterInformationTextSecond(server)
			{}

			template<atcmd::server::concepts::HexadecimalNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base = 16) &&
			{
				static_assert(std::same_as<P, T>, "Wrong order of parameters");
				printNumericParameter_(value, base);
				// TODO probably a compiler bug
				return atcmd::server::detail::ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecondTmpl<atcmd::server::detail::ExtendedCommandBase::ParameterList<Ts...>>(m_server);
				//return ParameterInformationTextSecondTmpl<ParameterList<Ts...>>(m_server);
			}

			template<atcmd::server::concepts::HexadecimalNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base) & = delete;
		};

		template<atcmd::server::concepts::BinaryNumericParameter T, class... Ts>
		struct ParameterInformationTextSecondTmpl<ParameterList<T, Ts...>> : private ParameterInformationTextSecond
		{
			explicit ParameterInformationTextSecondTmpl(Server& server) :
				ParameterInformationTextSecond(server)
			{}

			template<atcmd::server::concepts::BinaryNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base = 2) &&
			{
				static_assert(std::same_as<P, T>, "Wrong order of parameters");
				printNumericParameter_(value, base);
				// TODO probably a compiler bug
				return atcmd::server::detail::ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecondTmpl<atcmd::server::detail::ExtendedCommandBase::ParameterList<Ts...>>(m_server);
				//return ParameterInformationTextSecondTmpl<ParameterList<Ts...>>(m_server);
			}

			template<atcmd::server::concepts::BinaryNumericParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printNumericParameter(uint32_t value, uint8_t base) & = delete;
		};

		template<atcmd::server::concepts::StringParameter T, class... Ts>
		struct ParameterInformationTextSecondTmpl<ParameterList<T, Ts...>> : private ParameterInformationTextSecond
		{
			explicit ParameterInformationTextSecondTmpl(Server& server) :
				ParameterInformationTextSecond(server)
			{}

			template<atcmd::server::concepts::StringParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printStringParameter(const char* s) &&
			{
				static_assert(std::same_as<P, T>, "Wrong order of parameters");
				printStringParameter_(s);
				// TODO probably a compiler bug
				return atcmd::server::detail::ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecondTmpl<atcmd::server::detail::ExtendedCommandBase::ParameterList<Ts...>>(m_server);
				//return ParameterInformationTextSecondTmpl<ParameterList<Ts...>>(m_server);
			}

			template<atcmd::server::concepts::StringParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printStringParameter(const char* s) & = delete;
		};

		template<atcmd::server::concepts::HexadecimalStringParameter T, class... Ts>
		struct ParameterInformationTextSecondTmpl<ParameterList<T, Ts...>> : private ParameterInformationTextSecond
		{
			explicit ParameterInformationTextSecondTmpl(Server& server) :
				ParameterInformationTextSecond(server)
			{}

			template<atcmd::server::concepts::HexadecimalStringParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printHexadecimalStringParameter(const uint8_t* data, uint16_t size) &&
			{
				static_assert(std::same_as<P, T>, "Wrong order of parameters");
				printHexadecimalStringParameter_(data, size);
				// TODO probably a compiler bug
				return atcmd::server::detail::ExtendedCommandBase::ReadServerHandle::ParameterInformationTextSecondTmpl<atcmd::server::detail::ExtendedCommandBase::ParameterList<Ts...>>(m_server);
				//return ParameterInformationTextSecondTmpl<ParameterList<Ts...>>(m_server);
			}

			template<atcmd::server::concepts::HexadecimalStringParameter P>
			ParameterInformationTextSecondTmpl<ParameterList<Ts...>>
			printHexadecimalStringParameter(const uint8_t* data, uint16_t size) & = delete;
		};

		ReadServerHandle(detail::Server& server, bool is_last_command);

	public:
		ParameterInformationText makeParameterInformationText(const char* name, bool is_result_code = false);

		template<class Pl>
		ParameterInformationTextTmpl<Pl> makeParameterInformationText(const char* name, bool is_result_code = false)
		{
			return ParameterInformationTextTmpl<Pl>(m_server, is_result_code, name, is_result_code && !m_is_last_command);
		}
	};

	using ReadMethod = atcmd::RESULT_CODE (*)(ReadServerHandle);
	using WriteMethod = atcmd::RESULT_CODE (*)(WriteServerHandle);
	using TestMethod = const char* (*)(TestServerHandle);
};

} /* namespace detail */


class ExtendedCommand : private detail::ExtendedCommandBase
{
protected:
	template<concepts::Parameter... Ts>
	using ParameterList = detail::ExtendedCommandBase::ParameterList<Ts...>;

	using TestServerHandle = detail::ExtendedCommandBase::TestServerHandle;
	using ReadServerHandle = detail::ExtendedCommandBase::ReadServerHandle;
	using WriteServerHandle = detail::ExtendedCommandBase::WriteServerHandle;

	using DecimalNumericParameter = detail::ExtendedCommandBase::DecimalNumericParameter;
	using HexadecimalNumericParameter = detail::ExtendedCommandBase::HexadecimalNumericParameter;
	using BinaryNumericParameter = detail::ExtendedCommandBase::BinaryNumericParameter;
	using StringParameter = detail::ExtendedCommandBase::StringParameter;
	using HexadecimalStringParameter = detail::ExtendedCommandBase::HexadecimalStringParameter;
};

namespace concepts {

template<class T>
concept ExtendedWriteCommand = requires
{
	{ static_cast<detail::ExtendedCommandBase::WriteMethod>(&T::Definition::onWrite) };
};

template<class T>
concept ExtendedReadCommand = requires
{
	{ static_cast<detail::ExtendedCommandBase::ReadMethod>(&T::Definition::onRead) };
};

template<class T>
concept ExtendedTestCommand = requires
{
	{ static_cast<detail::ExtendedCommandBase::TestMethod>(&T::Definition::onTest) };
};

template<class T>
concept ExtendedCommand =
	detail::concepts::Command<T> &&
	std::convertible_to<T, atcmd::server::ExtendedCommand> &&
	(sizeof(T::Definition::name) > 1) &&
	atcmd::detail::Characters::isUpperAlphabetic(T::Definition::name[0]) &&
	(ExtendedWriteCommand<T> || ExtendedReadCommand<T>);

} /* namespace concepts */

} /* namespace atcmd::server */

#endif // ATCMD_EXTENDEDCOMMAND_H
