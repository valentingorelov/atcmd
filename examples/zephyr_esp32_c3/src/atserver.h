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

#ifndef ATSERVER_H
#define ATSERVER_H

#include <zephyr/kernel.h>

#include "commands/extended/rgb.h"
#include <atcmd/server/server.h>

struct ServerSettings
{
    using BasicCommands = atcmd::server::BasicCommandList<>;
    using AmpersandCommands = atcmd::server::AmpersandCommandList<>;
    using ExtendedCommands = atcmd::server::ExtendedCommandList<Rgb>;

    static constexpr std::size_t max_commands_per_line = 3;
};

struct AtServer : public atcmd::server::Server<ServerSettings>
{
    AtServer(k_pipe& rx_pipe, k_poll_event& rx_event, k_pipe& tx_pipe);

    void processRxData();

private:
    static void printCharCallback(char ch, void* context);

    k_pipe& m_rx_pipe;
    k_poll_event& m_rx_event;

    k_pipe& m_tx_pipe;
};

#endif // ATSERVER_H
