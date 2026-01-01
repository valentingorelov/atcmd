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

#ifndef BASICCMDDEF_H
#define BASICCMDDEF_H

#include <atcmd/detail/cmdparamdef.h>
#include <atcmd/server/basiccommand.h>

namespace atcmd::server::detail {

struct BasicCmdDef : public CmdParamDef
{
	const Ranges* numeric_ranges;
	BasicCommandBase::ExecMethod exec_method;
	char name;

	template<class Parameter>
	struct ParameterBuilderBase
	{
		static constexpr Ranges ranges =
		{
			.count = sizeof(Parameter::ranges) / sizeof(Parameter::ranges[0]),
			.ranges = Parameter::ranges
		};
	};

	struct ParameterBuilderStub
	{
		static constexpr Ranges ranges =
		{
			.count = 0,
			.ranges = nullptr
		};
	};

	template<class T>
	struct ParameterBuilder;

	template<class... T>
	struct ParameterBuilder<atcmd::server::BasicCommand::ParameterList<T...>> :
		public std::conditional_t<(sizeof...(T) > 0), ParameterBuilderBase<T...>, ParameterBuilderStub>
	{};

	template<class AtCmd>
	static consteval BasicCmdDef build()
	{
		BasicCmdDef r;
		if constexpr (ParameterBuilder<typename AtCmd::Definition::Parameters>::ranges.count != 0)
		{
			r.numeric_ranges = &ParameterBuilder<typename AtCmd::Definition::Parameters>::ranges;
		}
		else
		{
			r.numeric_ranges = nullptr;
		}
		r.exec_method = AtCmd::Definition::onExec;
		r.name = AtCmd::Definition::name[0];

		return r;
	}
};

} /* namespace atcmd::server::detail */

#endif // BASICCMDDEF_H
