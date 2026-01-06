#include "Dri/Dri_WS2812.h"
#include "esp_log.h"
#include "led_strip.h" // 引用刚刚下载的库

static const char *TAG = "RGB_LED";

// === 配置区 ===
#define LED_STRIP_GPIO_PIN      6     // 你的 DIN 接在这里 (P06)
#define LED_STRIP_LED_NUMBERS   1     // 你只有 1 颗灯珠
#define LED_STRIP_RMT_RES_HZ    (10 * 1000 * 1000) // 10MHz 分辨率

static led_strip_handle_t led_strip; // 灯带句柄

void RGB_Led_Init(void)
{
    // 配置 RMT 后端 (专门用于 WS2812)
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO_PIN,
        .max_leds = LED_STRIP_LED_NUMBERS,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RMT_RES_HZ,
        .flags.with_dma = false, // 一颗灯不需要 DMA，省资源
    };

    // 创建句柄
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    
    // 初始化后先关灯
    led_strip_clear(led_strip);
    ESP_LOGI(TAG, "WS2812 Init on GPIO %d", LED_STRIP_GPIO_PIN);
}

void RGB_Set_Color(uint8_t r, uint8_t g, uint8_t b)
{
    if (led_strip) {
        // 设置第 0 颗灯珠的颜色
        led_strip_set_pixel(led_strip, 0, r, g, b);
        // 发送数据刷新
        led_strip_refresh(led_strip);
    }
}