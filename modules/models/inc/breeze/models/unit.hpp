/**
 * @file unit.hpp
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 对数值 "单位" 的语义建模
 * @version 0.1
 * @date 2026-10-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <cstdint>

namespace breeze::models
{

/**
 * @brief 整数百分比
 *
 */
struct Percentage
{
	std::int8_t Value; ///< 整数值, 可正可负, 绝对值 <= 100
};

} // namespace breeze::models
