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

#ifndef COMMAND_BASE_H
#define COMMAND_BASE_H

#include <concepts>
#include <cstdint>

#include <atcmd/detail/cmdparamdef.h>

namespace atcmd::server {

namespace detail {

class Server;

struct CommandBase
{
	struct ServerHandle
	{
		Server& getServer();

	protected:
		struct InformationText
		{
			InformationText(Server& server, bool is_result_code);
			~InformationText();

			void printText(const char* text) &&;
			void printText(const char* text) & = delete;

		protected:
			Server& m_server;
			bool m_is_result_code;
		};

		ServerHandle(Server& server, bool is_last_command);

		Server& m_server;
		bool m_is_last_command;

	public:
		InformationText makeInformationText();
	};

	struct ParamServerHandle
	{
		ParamServerHandle(const uint8_t* param_start);

		const uint8_t* getParamStart() const;

	private:
		const uint8_t* m_param_start;
	};

	struct Parameter
	{
		Parameter() = delete;
		Parameter(const Parameter&) = delete;
		Parameter(Parameter&&) = delete;
	};

	struct NumericParameter :
			public Parameter,
			private CmdParamDef
	{
		using Range = CmdParamDef::Range;
	};
};

namespace concepts {

template<class T>
concept Command =
	// name exists and is usable as const char*
	requires { { T::Definition::name } -> std::convertible_to<const char*>; } &&

	// name is specifically a const char array
	std::is_array_v<decltype(T::Definition::name)> &&
	std::is_same_v<std::remove_extent_t<decltype(T::Definition::name)>, const char> &&

	// non-empty and null-terminated
	(T::Definition::name[0] != '\0') &&
	(T::Definition::name[sizeof(T::Definition::name) - 1] == '\0');

} /* namespace concepts */
} /* namespace detail */

namespace concepts {

template<class T>
concept Parameter =
	std::derived_from<T, detail::CommandBase::Parameter>;

template<class T>
concept NumericParameter =
	std::derived_from<T, detail::CommandBase::NumericParameter> &&
	requires
	{
		{ T::ranges } -> std::convertible_to<const typename T::Range(&)[]>;
	};
} /* namespace concepts */

namespace detail {

struct Command : public CommandBase
{
	struct ParameterListBase
	{
		ParameterListBase(const uint8_t* params);

	protected:
		uint32_t getNumeric_(std::size_t offset) const;

		const uint8_t* m_params;
	};

	template<atcmd::server::concepts::Parameter... Ts>
	class ParameterList : public ParameterListBase
	{
	protected:
		template<atcmd::server::concepts::Parameter P, atcmd::server::concepts::Parameter... Ps>
		struct OffsetCalc;

		template<atcmd::server::concepts::Parameter P, atcmd::server::concepts::Parameter... Ps>
		struct OffsetCalc<P, P, Ps...>
		{
			static constexpr std::size_t offset = 0;
		};

		template<atcmd::server::concepts::Parameter P, atcmd::server::concepts::NumericParameter N, atcmd::server::concepts::Parameter... Ps>
		struct OffsetCalc<P, N, Ps...>
		{
			static constexpr std::size_t offset = sizeof(uint32_t) + OffsetCalc<P, Ps...>::offset;
		};

	public:
		ParameterList(const ParamServerHandle& handle) :
			ParameterListBase(handle.getParamStart())
		{}

		template<atcmd::server::concepts::NumericParameter N>
		uint32_t getNumeric() const
		{
			return getNumeric_(OffsetCalc<N, Ts...>::offset);
		}
	};
};

} /* namespace detail */

} /* namespace atcmd::server */

#endif // COMMAND_BASE_H
