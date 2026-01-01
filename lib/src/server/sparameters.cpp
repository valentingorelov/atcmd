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

#include <atcmd/server/sparameters.h>

namespace atcmd::server {

SParameters::SParameters() :
	m_s3{'\r'},
	m_s4{'\n'},
	m_verbose{true}
{}

char SParameters::getCmdLineTerminationChar() const
{
	return m_s3;
}

void SParameters::setCmdLineTerminationChar(char ch)
{
	m_s3 = ch;
}

char SParameters::getResponseFormattingChar() const
{
	return m_s4;
}

void SParameters::setResponseFormattingChar(char ch)
{
	m_s4 = ch;
}

bool SParameters::isVerbose() const
{
	return m_verbose;
}

void SParameters::setVerbose(bool verbose)
{
	m_verbose = verbose;
}

}
