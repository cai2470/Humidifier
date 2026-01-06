#include "App/App_wifi.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

// 引用驱动和模块
#include "Dri/humidifier.h" // 控制加湿器
#include "App/App_led.h"    // <--- 核心：引用灯光控制头文件

static const char *TAG = "APP_WIFI";

// === Wi-Fi 配置信息 ===
#define WIFI_SSID      "Humidifier_WiFi"
#define WIFI_PASS      "12345678"
#define PORT           3333

// TCP 服务器任务
static void tcp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    struct sockaddr_in6 dest_addr;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);

    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    listen(listen_sock, 1);

    ESP_LOGI(TAG, "Socket listening on port %d", PORT);

    while (1) {
        struct sockaddr_storage source_addr;
        socklen_t addr_len = sizeof(source_addr);
        
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket accepted");

        while (1) {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            } else if (len == 0) {
                ESP_LOGI(TAG, "Connection closed");
                break;
            } else {
                rx_buffer[len] = 0;
                ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

                // ==========================================================
                // ? 遥控指令解析区
                // ==========================================================
                
                // 【指令 0】关机 (关雾 + 关灯)
                if (rx_buffer[0] == '0') {
                    Humidifier_Stop();
                    App_Rgb_SetMode(RGB_MODE_OFF); // 切换到关闭模式
                    send(sock, "ALL OFF\n", 8, 0);
                }
                
                // 【指令 1】开机 + 彩虹流光
                else if (rx_buffer[0] == '1') {
                    Humidifier_Start();
                    App_Rgb_SetMode(RGB_MODE_RAINBOW); // 切换到彩虹模式
                    send(sock, "Mode: RAINBOW\n", 14, 0);
                }

                // 【指令 2】开机 + 深海呼吸
                else if (rx_buffer[0] == '2') {
                    Humidifier_Start();
                    App_Rgb_SetMode(RGB_MODE_BREATHING); // 切换到呼吸模式
                    send(sock, "Mode: BREATHING\n", 16, 0);
                }

                // 【指令 3】开机 + 赛博朋克
                else if (rx_buffer[0] == '3') {
                    Humidifier_Start();
                    App_Rgb_SetMode(RGB_MODE_CYBERPUNK); // 切换到赛博模式
                    send(sock, "Mode: CYBERPUNK\n", 16, 0);
                }
                
                // 如果收到其他指令，可以回一个提示
                else {
                    send(sock, "Unknown Command. Try 0,1,2,3\n", 29, 0);
                }
            }
        }
        shutdown(sock, 0);
        close(sock);
    }
    vTaskDelete(NULL);
}

// 启动 Wi-Fi AP 模式
static void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = 1,
            .password = WIFI_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };
    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP Started. SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
}

void App_Wifi_Init(void)
{
    wifi_init_softap();
    xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);
}