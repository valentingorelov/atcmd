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

#ifndef ATCMD_CMDPARAMDEF_H
#define ATCMD_CMDPARAMDEF_H

#include <cstdint>

namespace atcmd::server::detail {

struct CmdParamDef
{
	struct Range
	{
		consteval Range(uint32_t min, uint32_t max) :
			m_min{min},
			m_max{max}
		{}

		uint32_t m_min;
		uint32_t m_max;
	};

	struct Ranges
	{
		uint8_t count;
		const Range* ranges;
	};

	static constexpr bool validateNumericRanges(const Ranges& numeric_ranges, const uint32_t& value)
	{
		for (uint_fast8_t i = 0; i < numeric_ranges.count; i++)
		{
			const auto& range = numeric_ranges.ranges[i];
			if ((value < range.m_min) || (value > range.m_max))
			{
				return false;
			}
		}
		return true;
	}
};

} /* namespace atcmd::server::detail */

#endif // ATCMD_CMDPARAMDEF_H
