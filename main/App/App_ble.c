#include "App_ble.h"
#include "Dri/Dri_ble.h"   // 引用驱动头文件
#include "Dri/humidifier.h" // 引用加湿器驱动
#include "esp_log.h"

static const char *TAG = "APP_BLE";

// --- 初始化函数 ---
void App_Ble_Init(void)
{
    ESP_LOGI(TAG, "Initializing BLE Application...");
    
    // 调用底层的驱动初始化
    Dri_BLE_Init();
}

// --- 核心：重写弱函数，处理接收到的数据 ---
// 当 Dri_ble.c 收到数据时，会自动跳到这里执行
void App_Communication_RecvDataCb(uint8_t *data, uint16_t dataLen)
{
    if (data == NULL || dataLen == 0) {
        return;
    }

    ESP_LOGI(TAG, "Recv Data Length: %d, CMD: 0x%02X", dataLen, data[0]);

    // 解析协议
    switch (data[0]) 
    {
        case 0xA1: // 打开指令
            ESP_LOGI(TAG, "Action: OPEN Humidifier");
            Humidifier_Start();
            break;

        case 0xA2: // 关闭指令
            ESP_LOGI(TAG, "Action: CLOSE Humidifier");
            Humidifier_Stop();
            break;

        default:
            ESP_LOGW(TAG, "Unknown Command: 0x%02X", data[0]);
            break;
    }
}