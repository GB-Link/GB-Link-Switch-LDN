#pragma once
#include "FreeRTOS.h"

typedef struct host_queue *QueueHandle_t;
QueueHandle_t xQueueCreate(uint32_t length, uint32_t item_size);
BaseType_t xQueueSend(QueueHandle_t queue, const void *item, TickType_t wait);
BaseType_t xQueueReceive(QueueHandle_t queue, void *item, TickType_t wait);
