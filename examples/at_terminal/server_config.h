/**
* Copyright © 2026 Valentin Gorelov
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

#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include <atcmd/server/server.h>

#include "commands/basic/i.h"
#include "commands/basic/v.h"

#include "commands/extended/gci.h"
#include "commands/extended/gmi.h"
#include "commands/extended/gmm.h"
#include "commands/extended/mv18am.h"
#include "commands/extended/test1dhb.h"
#include "commands/extended/test2sds.h"
#include "commands/extended/test3rsr.h"
#include "commands/extended/test4async.h"

struct ServerSettings
{
	using BasicCommands = atcmd::server::BasicCommandList<I, V>;
	using AmpersandCommands = atcmd::server::AmpersandCommandList<V>;
	using ExtendedCommands = atcmd::server::ExtendedCommandList<
		Gci,
		Gmi,
		Gmm,
		Mv18am,
		Test1dhb,
		Test2sds,
		Test3rsr,
		Test4async
	>;

	static constexpr std::size_t max_commands_per_line = 3;
};

extern atcmd::server::Server<ServerSettings> server;

#endif // SERVER_CONFIG_H
