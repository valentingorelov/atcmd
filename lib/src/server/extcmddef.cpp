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

#include <atcmd/detail/extcmddef.h>

namespace atcmd::server::detail {

ExtendedCommandBase::ReadMethod ExtCmdDef::getReadMethod() const
{
	if (!m_flags.readable)
	{
		return nullptr;
	}

	if (m_flags.single_method)
	{
		return m_methods.method.read;
	}
	return m_methods.methods[0].read;
}

ExtendedCommandBase::WriteMethod ExtCmdDef::getWriteMethod() const
{
	if (!m_flags.writable)
	{
		return nullptr;
	}

	if (m_flags.single_method)
	{
		return m_methods.method.write;
	}
	return m_methods.methods[1 - !m_flags.readable].write;
}

ExtendedCommandBase::TestMethod ExtCmdDef::getTestMethod() const
{
	if (!m_flags.custom_testable)
	{
		return nullptr;
	}

	if (m_flags.single_method)
	{
		return m_methods.method.test;
	}
	return m_methods.methods[2 - !m_flags.readable - !m_flags.writable].test;
}

ExtendedCommandBase::AbortMethod ExtCmdDef::getAbortMethod() const
{
	if (!m_flags.abortable)
	{
		return nullptr;
	}

	if (m_flags.single_method)
	{
		return m_methods.method.abort;
	}
	return m_methods.methods[3 - !m_flags.readable - !m_flags.writable - !m_flags.custom_testable].abort;
}

} /* atcmd::server::detail */
