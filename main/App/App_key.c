#include "App_key.h"
#include "Dri/Dri_key.h"         // 引用按键驱动
#include "Dri/humidifier.h"  // 引用加湿器驱动
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "App/App_led.h"     // 引用 RGB 灯效应用

static const char *TAG = "APP_KEY";

// 定义一个变量来记录当前开关状态 (0:关, 1:开)
static int is_humidifier_on = 0;

// --- 按键扫描任务 ---
void Key_Task(void *arg)
{
    while (1) {
        // 1. 检测按键是否按下 (低电平有效)
        if (Key_Get_Level() == 0) {
            
            // 2. 延时消抖 (关键步骤！过滤掉机械抖动)
            vTaskDelay(pdMS_TO_TICKS(20));

            // 3. 再次确认是否真的按下了
            if (Key_Get_Level() == 0) {
                ESP_LOGI(TAG, "Key Pressed!");

                // 4. 执行翻转逻辑 (开变关，关变开)
                if (is_humidifier_on == 0) {
                    Humidifier_Start();
                    App_Rgb_Start();
                    is_humidifier_on = 1;
                    ESP_LOGI(TAG, "Action: ON");
                } else {
                    Humidifier_Stop();
                    App_Rgb_Stop();
                    is_humidifier_on = 0;
                    ESP_LOGI(TAG, "Action: OFF");
                }

                // 5. 等待按键松开 (防止长按一直触发)
                while (Key_Get_Level() == 0) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                ESP_LOGI(TAG, "Key Released");
            }
        }

        // 6. 释放 CPU 资源 (10ms 扫描一次足矣)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void App_Key_Init(void)
{
    // 初始化硬件
    Key_Init();

    // 创建按键扫描任务
    // 栈大小 2048 字节，优先级 5
    xTaskCreate(Key_Task, "Key_Task", 2048, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Key Task Started");
}