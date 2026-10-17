/**
 * @file referee.h
 * @author vix_hentx (w1084349470@outlook.com)
 * @brief 裁判系统协议. 适用于2026赛季, 2.0.0协议.
 * @version 26.2.0
 * @date 2026-10-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once

#include <cstdint>

namespace breeze::referee::protocol
{

constexpr uint8_t sof = 0xa5;

#pragma pack(push,1)
/**
 * @brief 裁判系统帧头
 * 
 */
struct frame_header
{
    uint8_t sof;
    uint16_t data_length;
    uint8_t seq;
    uint8_t crc8;
};
#pragma pack(pop)
static_assert(sizeof(frame_header) == 5, "referee frame header size mismatch");

namespace rx
{
    
}

}