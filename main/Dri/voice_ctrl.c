#include "voice_ctrl.h"
#include "humidifier.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>
#include "App/App_led.h"

static const char *TAG = "VOICE_CTRL";
static const int RX_BUF_SIZE = 1024;

void Voice_Ctrl_Init(void) {
    const uart_config_t uart_config = {
        .baud_rate = VOICE_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // 安装驱动 (使用 UART_NUM_0)
    // 注意：这里安装驱动后，原本的系统日志打印功能就会被彻底覆盖
    uart_driver_install(VOICE_UART_PORT, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(VOICE_UART_PORT, &uart_config);
    
    // 使用默认引脚 (21/20)
    uart_set_pin(VOICE_UART_PORT, VOICE_UART_TX_PIN, VOICE_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    // 因为日志通道已经改到了 USB-JTAG，这里打印日志是安全的，会在电脑上显示
    ESP_LOGI(TAG, "Voice UART (UART0) Init Done.");
     // 确保 Dri/voice_ctrl.c 里有 Voice_Task 这个函数
    xTaskCreate(Voice_Task, "voice_task", 4096, NULL, 5, NULL);
}

void Voice_Task(void *arg) {
    uint8_t *data = (uint8_t *) malloc(RX_BUF_SIZE);
    
    while (1) {
        int len = uart_read_bytes(VOICE_UART_PORT, data, RX_BUF_SIZE, pdMS_TO_TICKS(100));
        
        if (len > 0) {
            // 调试打印接收到的数据
            ESP_LOGI(TAG, "Recv: 0x%02X", data[0]);
            
            switch (data[0]) {
                // 指令 0xA1: "打开加湿器"
                case 0xA1: // 或者你原来的 0xA1
                    ESP_LOGI(TAG, "CMD: Turn ON");
                    Humidifier_Start();                 // 开喷雾
                    App_Rgb_SetMode(RGB_MODE_RAINBOW);  // 开灯(默认彩虹)
                    break;

                // 指令 0xA2: "关闭加湿器"
                case 0xA2: // 或者你原来的 0xA2
                    ESP_LOGI(TAG, "CMD: Turn OFF");
                    Humidifier_Stop();                  // 关喷雾
                    App_Rgb_SetMode(RGB_MODE_OFF);      // 关灯
                    break;

                // 指令 0xA3: "打开呼吸灯" / "切换呼吸模式"
                case 0xA3:
                    ESP_LOGI(TAG, "CMD: Breathing Mode");
                    // 这里不强制开加湿器，只切换灯光，显得更灵活
                    // 如果你想联动，可以加上 Humidifier_Start();
                    App_Rgb_SetMode(RGB_MODE_BREATHING);
                    break;

                // 指令 0xA4: "打开氛围灯" / "切换赛博模式"
                case 0xA4:
                    ESP_LOGI(TAG, "CMD: Cyberpunk Mode");
                    App_Rgb_SetMode(RGB_MODE_CYBERPUNK);
                    break;

                // 指令 0xA5: "变颜色" / "切换彩虹模式"
                case 0xA5:
                    ESP_LOGI(TAG, "CMD: Rainbow Mode");
                    App_Rgb_SetMode(RGB_MODE_RAINBOW);
                    break;

                default:
                    ESP_LOGW(TAG, "Unknown CMD: 0x%02X", data[0]);
                    break;
            }
        }
    }
    free(data);
}