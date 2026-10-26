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

#pragma once

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

/**
 * @brief 角度. 不加说明则规定为顺时针转动, 范围 0~360度.
 *
 * @tparam T
 */
template <typename T>
struct Degree
{
	T Value;
};

/**
 * @brief 弧度. 不佳说明则规定为逆时针转动, 单位radian.
 *
 * @tparam T
 */
template <typename T>
struct Radian
{
	T Value;
};

/**
 * @brief 2D史亮
 *
 */
template <typename T>
struct Vec2
{
	T X, Y;
};

/**
 * @brief 3D史亮
 *
 */
template <typename T>
struct Vec3
{
	T X, Y, Z;
};

} // namespace breeze::models
