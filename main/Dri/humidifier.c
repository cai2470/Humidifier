#include "humidifier.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h" 

static const char *TAG = "HUMIDIFIER_DRI";

void Humidifier_Init(void)
{
    ESP_LOGI(TAG, "Initializing Hardware...");

    // 1. 救命招：关闭掉电检测 (顺序完全一致)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // 2. 防误触拉低 
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;     // 禁用中断
    io_conf.mode = GPIO_MODE_OUTPUT;    // 输出模式 
    io_conf.pin_bit_mask = (1ULL << HUMIDIFIER_PIN);   //使用的引脚
    io_conf.pull_down_en = 1;          // 使能下拉
    io_conf.pull_up_en = 0;            //不使能上拉
    gpio_config(&io_conf);
    gpio_set_level(HUMIDIFIER_PIN, 0);   

    // 3. 这里的延时非常关键！
    vTaskDelay(pdMS_TO_TICKS(100));

    // 4. 配置 PWM 定时器
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .freq_hz          = HUMIDIFIER_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // 5. 配置 PWM 通道
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = HUMIDIFIER_PIN,
        .duty           = 0, 
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    
    ESP_LOGI(TAG, "Init Done.");
}

void Humidifier_Start(void)
{
    ESP_LOGI(TAG, "Starting Soft-Start...");

    // 软启动逻辑
    for (int duty = 0; duty <= MAX_DUTY; duty++) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
    ESP_LOGI(TAG, "Continuous Mist Mode ON.");
}

// --- 新增的 Stop 函数 ---
void Humidifier_Stop(void)
{
    ESP_LOGI(TAG, "Stopping (Soft-Stop)...");

    // 软停止：慢慢减小油门，防止电流突然消失导致电压反冲
    for (int duty = MAX_DUTY; duty >= 0; duty--) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        // 这里的延时比启动稍微短一点，关机可以稍微快点，但不能瞬间关
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }

    // 双重保险：确保最后一定是 0
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    
    ESP_LOGI(TAG, "Mist OFF.");
}