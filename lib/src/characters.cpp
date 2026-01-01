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

#include <atcmd/detail/characters.h>

namespace atcmd::detail {

static constexpr char numbers[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

char Characters::toUpper(char ch)
{
	if ((ch >= 'a') && (ch <= 'z'))
	{
		ch -= 'a' - 'A';
	}
	return ch;
}

void Characters::printNumber(uint32_t number, uint8_t base, PrintCharCallback callback, void* user_handle)
{
	assert(base <= (sizeof(numbers)));

	uint_fast8_t digit_count = 0;
	uint64_t number_reversed = 0;
	do
	{
		number_reversed *= base;
		number_reversed += number % base;
		number /= base;
		digit_count++;
	} while (number != 0);

	do
	{
		callback(numbers[number_reversed % base], user_handle);
		number_reversed /= base;
	} while (--digit_count != 0);
}

void Characters::printHexadecimalString(const uint8_t* data, uint16_t size, PrintCharCallback callback, void* user_handle)
{
	for (uint_fast16_t i = 0; i < size; i++)
	{
		uint8_t v = data[i];
		callback(numbers[v >> 4], user_handle);
		callback(numbers[v & 0xF], user_handle);
	}
}

} /* namespace atcmd::detail */
