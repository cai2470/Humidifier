#ifndef _HUMIDIFIER_H_
#define _HUMIDIFIER_H_

#ifdef __cplusplus
extern "C" {
#endif

// 保持和 main.c 一模一样的参数
#define HUMIDIFIER_PIN      3
#define HUMIDIFIER_FREQ     108000
#define MAX_DUTY            100 

void Humidifier_Init(void);
void Humidifier_Start(void);
void Humidifier_Stop(void);

#ifdef __cplusplus
}
#endif

#endif