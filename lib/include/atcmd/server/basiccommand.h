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

#ifndef ATCMD_BASICCOMMAND_H
#define ATCMD_BASICCOMMAND_H

#include <atcmd/common.h>
#include <atcmd/detail/characters.h>
#include <atcmd/server/command_base.h>

namespace atcmd::server {

namespace detail {

struct BasicCommandBase : protected Command
{
	struct BasicServerHandle :
			public ServerHandle,
			public ParamServerHandle
	{
		friend class Server;

	private:
		BasicServerHandle(const uint8_t* param_start, Server& server, bool is_last_command, CALL_TYPE call_type);
	};

	struct BasicNumericParameter : public NumericParameter
	{};

	using ExecMethod = atcmd::RESULT_CODE (*)(BasicServerHandle);
};

} /* namespace detail */

namespace concepts {

template<class T>
concept BasicNumericParameter =
	NumericParameter<T> &&
	std::derived_from<T, detail::BasicCommandBase::BasicNumericParameter> &&
	!requires{T::is_optional;};

} /* namespace concepts */

struct BasicCommand : private detail::BasicCommandBase
{
protected:
	template<concepts::BasicNumericParameter... Ts>
	struct ParameterList : public detail::BasicCommandBase::ParameterList<Ts...>
	{
		static_assert(sizeof...(Ts) <= 1, "Basic commands can have no more than 1 numeric parameter");
		using detail::BasicCommandBase::ParameterList<Ts...>::ParameterList;
	};

	using BasicServerHandle = detail::BasicCommandBase::BasicServerHandle;
	using BasicNumericParameter = detail::BasicCommandBase::BasicNumericParameter;
};

namespace concepts {

template<class T>
concept AmpersandCommand =
	detail::concepts::Command<T> &&
	std::convertible_to<T, atcmd::server::BasicCommand> &&
	(sizeof(T::Definition::name) == 2) &&
	atcmd::detail::Characters::isUpperAlphabetic(T::Definition::name[0]);

template<class T>
concept BasicCommand =
	AmpersandCommand<T> &&
	(T::Definition::name[0] != 'S');

} /* namespace concepts */

} /* namespace atcmd::server */

#endif // ATCMD_BASICCOMMAND_H
