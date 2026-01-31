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

#include <gtest/gtest.h>

#include <atcmd/detail/trie.h>

constexpr std::size_t NAME_MAX_SIZE = 10;
constexpr std::size_t NAMES_COUNT = 16383;

struct Name
{
	char name[NAME_MAX_SIZE + 1];
};

typedef std::array<Name, NAMES_COUNT> Names;


constexpr Name getNextName(const Name& current_name)
{
	Name r = current_name;
	char* name = r.name;
	std::size_t pos = 0;

	while (true)
	{
		if (name[pos] == 'Z')
		{
			name[pos] = 'A';
			pos++;
			continue;
		}
		if (name[pos] == '\0')
		{
			name[pos] = 'A';
			return r;
		}
		name[pos]++;
		return r;
	}
}

consteval Names generateNames()
{
	Names r{"A"};

	for (std::size_t i = 1; i < r.size(); i++)
	{
		r[i] = getNextName(getNextName(r[i - 1]));
	}

	return r;
}

constexpr Names l_names = generateNames();

consteval std::array<const char*, NAMES_COUNT> getNames()
{
	std::array<const char*, NAMES_COUNT> r{};

	for (std::size_t i = 0; i < NAMES_COUNT; i++)
	{
		r[i] = l_names[i].name;
	}

	return r;
}

static atcmd::detail::TrieBase<NAMES_COUNT, getNames()> m_trie;
static auto names_ = getNames();

TEST(TrieTest, Full) {
	Name current_name{};

	for (std::size_t i = 0; i < NAMES_COUNT * 2; i++)
	{
		bool present = !(i & 0x01);
		m_trie.reset();
		current_name = getNextName(current_name);
		const char* ch = &current_name.name[0];
		while (*ch != '\0')
		{
			if (!m_trie.feed(*ch))
			{
				ASSERT_FALSE(present);
				break;
			}
			ch++;
		}
		if (*ch == '\0')
		{
			if (present)
			{
				ASSERT_TRUE(m_trie.isLeaf());
				ASSERT_EQ(i / 2, m_trie.getCommandIndex());
			}
			else
			{
				ASSERT_FALSE(m_trie.isLeaf());
			}
		}
	}
}

constexpr const char ab[] = "AB";
static atcmd::detail::TrieBase<1, {ab}> m_trie_single;

TEST(TrieTest, NoFirstChild) {
	m_trie_single.reset();
	ASSERT_TRUE(m_trie_single.feed('A'));
	ASSERT_TRUE(m_trie_single.feed('B'));
	ASSERT_FALSE(m_trie_single.feed('C'));
}

#ifdef __clang__
// Clang crashes when this value is high
constexpr std::size_t LONG_NAME_SIZE = 1000;
#else
constexpr std::size_t LONG_NAME_SIZE = 10000;
#endif

struct LongName
{
	char name[LONG_NAME_SIZE + 1];
};

consteval LongName getLongName()
{
	LongName r{};

	for (std::size_t i = 0; i < LONG_NAME_SIZE; i++)
	{
		r.name[i] = 'A';
	}

	return r;
}

constexpr const LongName long_name = getLongName();
static atcmd::detail::TrieBase<1, {long_name.name}> m_trie_long;

TEST(TrieTest, LongName) {
	m_trie_long.reset();

	for (std::size_t i = 0; i < LONG_NAME_SIZE; i++)
	{
		ASSERT_TRUE(m_trie_long.feed('A'));
	}
	ASSERT_TRUE(m_trie_long.isLeaf());
}
