#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>

// 初始化 WS2812
void RGB_Led_Init(void);

// 设置颜色 (R, G, B: 0-255)
void RGB_Set_Color(uint8_t r, uint8_t g, uint8_t b);

#endif