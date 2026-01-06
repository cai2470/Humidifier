#ifndef _VOICE_CTRL_H_
#define _VOICE_CTRL_H_

#include "driver/uart.h"

// --- 强制使用 UART0 (也就是 U0TXD/U0RXD) ---
#define VOICE_UART_PORT    UART_NUM_0 
#define VOICE_UART_BAUD    9600

// ESP32-C3 的 UART0 默认引脚通常是 GPIO 21 (TX) 和 GPIO 20 (RX)
// 我们使用 UART_PIN_NO_CHANGE 让驱动自动使用默认引脚
#define VOICE_UART_TX_PIN  UART_PIN_NO_CHANGE
#define VOICE_UART_RX_PIN  UART_PIN_NO_CHANGE

void Voice_Ctrl_Init(void);
void Voice_Task(void *arg);

#endif