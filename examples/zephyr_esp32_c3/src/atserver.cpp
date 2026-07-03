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

#include "atserver.h"

AtServer::AtServer(k_pipe& rx_pipe, k_poll_event& rx_event, k_pipe& tx_pipe) :
    Server{printCharCallback, this},
    m_rx_pipe{rx_pipe},
    m_rx_event{rx_event},
    m_tx_pipe{tx_pipe}
{
}

void AtServer::processRxData()
{
    if (m_rx_event.state == K_POLL_STATE_PIPE_DATA_AVAILABLE)
    {
        uint8_t data;
        while (k_pipe_read(&m_rx_pipe, &data, 1, K_NO_WAIT) > 0)
        {
            feed(static_cast<char>(data));
        }
    }
}

void AtServer::printCharCallback(char ch, void* context)
{
    uint8_t vl = ch;
    AtServer* server = static_cast<AtServer*>(context);
    k_pipe_write(&server->m_tx_pipe, &vl, 1, K_NO_WAIT);
}
