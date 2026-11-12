#pragma once
#include "FreeRTOS.h"

typedef void *TaskHandle_t;
TickType_t xTaskGetTickCount(void);
void vTaskDelay(TickType_t ticks);
BaseType_t xTaskCreate(void (*task)(void *), const char *name, uint32_t stack, void *arg, int priority,
                       TaskHandle_t *handle);
void vTaskDelete(TaskHandle_t task);
