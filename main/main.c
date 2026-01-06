#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

// ==========================================
// 1. 引入各模块头文件
// ==========================================
#include "Dri/humidifier.h"  // 加湿器驱动
#include "Dri/voice_ctrl.h"  // 语音模块驱动
#include "App/App_key.h"     // 按键业务逻辑
#include "App/App_wifi.h"    // Wi-Fi 和 TCP 控制
#include "App/App_led.h"     // 炫酷 RGB 灯效
#include "App/App_ble.h"     // 蓝牙模块

static const char *TAG = "MAIN";

void app_main(void)
{
    // ============================================================
    // 第一步：NVS 初始化 (Wi-Fi 功能必须)
    // ============================================================
    // NVS 用于存储 Wi-Fi 的底层配置信息，必须最先初始化
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // 如果 NVS 分区满了或者版本不对，执行擦除并重新初始化
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // ============================================================
    // 第二步：底层硬件初始化
    // ============================================================
    ESP_LOGI(TAG, "Initializing Hardware...");
    
    Humidifier_Init();  // 初始化雾化片控制引脚
    Voice_Ctrl_Init();  // 初始化语音模块串口
    
    // ============================================================
    // 第三步：应用层模块初始化
    // ============================================================
    ESP_LOGI(TAG, "Initializing Apps...");

    App_Key_Init();     // 启动物理按键检测任务
    
    App_Rgb_Init();     // 启动 RGB 灯光任务 (默认是熄灭状态)
                        // 注意：具体的"开灯/关灯"逻辑在 App_wifi.c 里调用

    App_Wifi_Init();    // 启动 Wi-Fi 热点和 TCP 监听服务器
    App_Ble_Init();    //   蓝牙模块

    // ============================================================
    // 第四步：启动语音识别任务
    // ============================================================
    // 确保 Dri/voice_ctrl.c 里有 Voice_Task 这个函数
    xTaskCreate(Voice_Task, "voice_task", 4096, NULL, 5, NULL);

    // ============================================================
    // 第五步：系统启动完成
    // ============================================================
    ESP_LOGI(TAG, "--------------------------------------------------");
    ESP_LOGI(TAG, "System Ready!");
    ESP_LOGI(TAG, "1. Connect WiFi: 'Humidifier_WiFi' (Pass: 12345678)");
    ESP_LOGI(TAG, "2. TCP Connect: 192.168.4.1 : 3333");
    ESP_LOGI(TAG, "3. Send '1' to turn ON (Spray + Rainbow LED)");
    ESP_LOGI(TAG, "4. Send '0' to turn OFF");
    ESP_LOGI(TAG, "--------------------------------------------------");

    // 主任务进入空闲循环，不退出
    while (1) {
        vTaskDelay(portMAX_DELAY);
    }
}