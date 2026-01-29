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

#ifndef ATCMD_CHARACTERS_H
#define ATCMD_CHARACTERS_H

#include <cstdint>
#include <array>
#include <assert.h>

#include <atcmd/common.h>

namespace atcmd::detail {

struct Characters
{
	Characters() = delete;

	static constexpr bool isUpperAlphabetic(char ch)
	{
		return (ch >= 'A') && (ch <= 'Z');
	}

	static constexpr bool isLowerAlphabetic(char ch)
	{
		return (ch >= 'a') && (ch <= 'z');
	}

	static constexpr bool isAlphabetic(char ch)
	{
		return isUpperAlphabetic(ch) || isLowerAlphabetic(ch);
	}

	static constexpr bool isNumeric(char ch)
	{
		return (ch >= '0') && (ch <= '9');;
	}

	static constexpr int_fast8_t getNumeric(char ch)
	{
		if (!isNumeric(ch))
		{
			return -1;
		}
		else
		{
			return ch - '0';
		}
	}

	static constexpr int_fast8_t getHex(char ch)
	{
		int_fast8_t r = getNumeric(ch);
		if (r == -1)
		{
			if ((ch >= 'a') && (ch <= 'f'))
			{
				r = 10 + (ch - 'a');
			}
			else if ((ch >= 'A') && (ch <= 'F'))
			{
				r = 10 + (ch - 'A');
			}
			else
			{
				r = -1;
			}
		}
		return r;
	}

	static char toUpper(char ch);

	static constexpr uint8_t encode(char ch)
	{
		static constexpr uint8_t alphabetic_offset = 'Z' - 'A' + 1;
		static constexpr uint8_t numeric_offset = alphabetic_offset + ('9' - '0' + 1);

		if (isUpperAlphabetic(ch))
		{
			return ch - 'A';
		}
		else if (isNumeric(ch))
		{
			return alphabetic_offset + (ch - '0');
		}
		else
		{
			switch (ch) {
			case '!':
				return numeric_offset;
			case '%':
				return numeric_offset + 1;
			case '-':
				return numeric_offset + 2;
			case '.':
				return numeric_offset + 3;
			case '/':
				return numeric_offset + 4;
			case ':':
				return numeric_offset + 5;
			case '_':
				return numeric_offset + 6;
			default:
				return 0xFF;
			}
		}
	}

	static constexpr char decode(uint8_t encoded)
	{
		assert(encoded < getAlphabetSize());
		return alphabet[encoded];
	}

	static void printNumber(uint32_t number, uint8_t base, PrintCharCallback callback, void* context);
	static void printHexadecimalString(const uint8_t* data, uint16_t size, PrintCharCallback callback, void* context);

	static constexpr uint_fast8_t getAlphabetSize()
	{
		return std::size(alphabet);
	}

private:
	static inline constexpr char alphabet[] =
	{
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'!', '%', '-', '.', '/', ':', '_'
	};
};

}

#endif // ATCMD_CHARACTERS_H
