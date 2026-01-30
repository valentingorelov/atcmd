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

#ifndef ATCMD_TRIE_H
#define ATCMD_TRIE_H

#include <atcmd/detail/triebuilder.h>
#include <atcmd/detail/characters.h>

namespace atcmd::detail {

template<const char*... names>
struct Trie
{
	Trie() : m_pos{0}
	{}

	void reset()
	{
		m_pos = 0;
	}

	bool feed(char ch)
	{
		// Goto the first child
		skipCommandIndex();
		if (getSubtreeSize() == 0)
		{
			return false;
		}

		while (true)
		{
			if (currentChar() == ch)
			{
				return true;
			}
			if (isLast())
			{
				m_pos = 0;
				return false;
			}
			// Go to the right sibling
			skipCommandIndex();
			uint32_t s = getSubtreeSize();
			m_pos += s;
		}
	}

	bool isLeaf() const
	{
		return current() & TrieBuilder::MASKS::MASKS_LEAF;
	}

	uint16_t getCommandIndex() const
	{
		assert(isLeaf());

		uint8_t b = m_trie[m_pos + 1];
		uint32_t r = b & 0x7F;
		if ((b & (1u << 7)) == 0)
		{
			return r;
		}
		r |= m_trie[m_pos + 2] << 7;
		return r;
	}

private:
	uint8_t current() const
	{
		return m_trie[m_pos];
	}

	char currentChar() const
	{
		return Characters::decode(current() & TrieBuilder::MASKS::MASKS_CHAR);
	}

	bool isLast() const
	{
		return current() & TrieBuilder::MASKS::MASKS_LAST;
	}

	void skipCommandIndex()
	{
		if (!isLeaf())
		{
			m_pos++;
			return;
		}

		m_pos++;
		if ((current() & (1u << 7)))
		{
			m_pos++;
		}
		m_pos++;
	}

	uint32_t getSubtreeSize()
	{
		uint32_t r = 0;

		// First 2 bytes
		for (uint_fast8_t i = 0; i < 2; i++)
		{
			r |= (current() & 0x7F) << (7 * i);
			if ((current() & (1u << 7)) == 0)
			{
				m_pos++;
				return r;
			}
			m_pos++;
		}

		// Third byte
		r |= current() << 14;
		m_pos++;
		return r;
	}

	std::size_t m_pos;

	static constexpr std::array<const char*, sizeof...(names)> m_names = {names...};
	static constexpr auto m_trie =
			TrieBuilder::getTrie<TrieBuilder::getTrieSize(m_names.data(), sizeof...(names))>(m_names.data(), sizeof...(names));
};

} /* namespace atcmd::detail */

#endif // ATCMD_TRIE_H
