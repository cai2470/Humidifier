#ifndef __DRI_KEY_H__
#define __DRI_KEY_H__

#include "driver/gpio.h"
// 定义按键引脚
#define KEY_GPIO_PIN    GPIO_NUM_2


// 初始化按键
void Key_Init(void);

// 读取按键状态 (返回 0 表示按下，1 表示松开)
int Key_Get_Level(void);

#endif /* __DRI_KEY_H__ */