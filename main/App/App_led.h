#ifndef APP_RGB_H
#define APP_RGB_H

typedef enum {
    RGB_MODE_OFF = 0,       // 关闭
    RGB_MODE_RAINBOW,       // 1. 彩虹流光
    RGB_MODE_BREATHING,     // 2. 冰蓝呼吸
    RGB_MODE_CYBERPUNK      // 3. 赛博朋克
} rgb_mode_t;


// 设置模式 (传入上面的枚举值)
void App_Rgb_SetMode(rgb_mode_t mode);

void App_Rgb_Init(void);
void App_Rgb_Start(void);
void App_Rgb_Stop(void);

#endif