#include "Dri_key.h"

void Key_Init(void)
{
    gpio_config_t io_conf = {};
    
    // 1. 设置为输入模式
    io_conf.intr_type = GPIO_INTR_DISABLE; // 禁用中断
    io_conf.mode = GPIO_MODE_INPUT;        // 输入模式
    io_conf.pin_bit_mask = (1ULL << KEY_GPIO_PIN); // 选择 GPIO 2
    io_conf.pull_down_en = 0;              // 禁用下拉
    io_conf.pull_up_en = 1;                // 启用上拉 (默认高电平，按下变低电平)
    
    gpio_config(&io_conf);
}

int Key_Get_Level(void)
{
    return gpio_get_level(KEY_GPIO_PIN);
}