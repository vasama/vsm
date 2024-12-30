#pragma once

#include <cstdint>

extern "C"
uint16_t vsm_detail_float_to_half(float x);

extern "C"
float vsm_detail_half_to_float(uint16_t x);
