#include "App/App_led.h"
#include "Dri/Dri_WS2812.h"    // 确保这里引用的是你底层 WS2812 驱动的头文件
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>           // 用于 sin() 函数计算呼吸灯
#include <stdlib.h>         // 用于 rand() 函数生成随机颜色

// 全局变量：记录当前运行的模式
static rgb_mode_t g_current_mode = RGB_MODE_OFF;

// ==========================================================
// ? 辅助算法：HSV 转 RGB
// ==========================================================
// 用于彩虹模式，将 0-360 的色相角转换为 RGB 数值
static void hsv2rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    float m = v - c;
    float r_temp, g_temp, b_temp;

    if (h >= 0 && h < 60) {
        r_temp = c; g_temp = x; b_temp = 0;
    } else if (h >= 60 && h < 120) {
        r_temp = x; g_temp = c; b_temp = 0;
    } else if (h >= 120 && h < 180) {
        r_temp = 0; g_temp = c; b_temp = x;
    } else if (h >= 180 && h < 240) {
        r_temp = 0; g_temp = x; b_temp = c;
    } else if (h >= 240 && h < 300) {
        r_temp = x; g_temp = 0; b_temp = c;
    } else {
        r_temp = c; g_temp = 0; b_temp = x;
    }

    *r = (uint8_t)((r_temp + m) * 255);
    *g = (uint8_t)((g_temp + m) * 255);
    *b = (uint8_t)((b_temp + m) * 255);
}

// ==========================================================
// ? 灯光主任务 (核心逻辑)
// ==========================================================
static void rgb_task(void *pvParameters)
{
    float hue = 0.0;           // 彩虹模式用的色相 (0.0 ~ 360.0)
    float breath_angle = 0.0;  // 呼吸模式用的角度 (0.0 ~ 2π)
    uint8_t r, g, b;

    while (1) {
        switch (g_current_mode) {
            
            // ? 模式 1：彩虹流光 (Rainbow)
            // 特点：颜色平滑过渡，红橙黄绿青蓝紫循环
            case RGB_MODE_RAINBOW:
                hsv2rgb(hue, 1.0, 0.5, &r, &g, &b); // 饱和度1.0，亮度0.5
                RGB_Set_Color(r, g, b);
                
                hue += 1.0; // 每次增加1度
                if (hue >= 360.0) hue = 0.0;
                
                vTaskDelay(pdMS_TO_TICKS(20)); // 刷新速度：中等 (50Hz)
                break;

            // ? 模式 2：深海呼吸 (Ocean Breathe)
            // 特点：只有冰蓝色，亮度像呼吸一样忽明忽暗
            case RGB_MODE_BREATHING:
                {
                    // 利用 sin 正弦波产生平滑的呼吸曲线 (-1 ~ 1 -> 0.0 ~ 1.0)
                    float brightness = (sin(breath_angle) + 1.0) / 2.0; 
                    
                    // 将亮度映射到 0~200 (最大亮度)
                    uint8_t val = (uint8_t)(brightness * 200);
                    
                    // 配色：冰蓝色 (R=0, G=少许, B=大量)
                    // 随着 val 变化，整体亮度会变化
                    RGB_Set_Color(0, val/3, val); 
                    
                    breath_angle += 0.05; // 呼吸速度
                    vTaskDelay(pdMS_TO_TICKS(20));
                }
                break;

            // ? 模式 3：赛博朋克 (Cyberpunk)
            // 特点：高饱和度颜色（紫、青、粉）随机快速跳变
            case RGB_MODE_CYBERPUNK:
                {
                    // 随机生成 0 或 1
                    int random_pick = rand() % 2;
                    
                    if (random_pick == 0) {
                        RGB_Set_Color(200, 0, 200); // 霓虹紫
                    } else {
                        RGB_Set_Color(0, 200, 200); // 赛博青
                    }
                    
                    // 刷新速度慢一点，产生闪烁感
                    vTaskDelay(pdMS_TO_TICKS(150)); 
                }
                break;

            // ? 关闭模式 (OFF)
            case RGB_MODE_OFF:
            default:
                RGB_Set_Color(0, 0, 0); // 熄灭
                vTaskDelay(pdMS_TO_TICKS(500)); // 低频检测，省电
                break;
        }
    }
}

// ==========================================================
// ? 对外接口
// ==========================================================

// 初始化
void App_Rgb_Init(void)
{
    // 1. 初始化底层硬件 (WS2812 RMT驱动)
    RGB_Led_Init();

    // 2. 创建灯光任务
    // 堆栈大小 2048 字节，优先级 5
    xTaskCreate(rgb_task, "rgb_task", 2048, NULL, 5, NULL);
}

// 设置模式
void App_Rgb_SetMode(rgb_mode_t mode)
{
    g_current_mode = mode;
}



// Start 默认开启彩虹模式
void App_Rgb_Start(void)
{
    g_current_mode = RGB_MODE_RAINBOW;
}

// Stop 直接关闭
void App_Rgb_Stop(void)
{
    g_current_mode = RGB_MODE_OFF;
}