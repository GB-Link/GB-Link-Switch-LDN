#pragma once
#include <stdio.h>
#include <stdlib.h>

typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_NO_MEM 0x101
#define ESP_ERROR_CHECK(x)                                                   \
    do {                                                                     \
        if ((x) != ESP_OK) {                                                 \
            fprintf(stderr, "ESP_ERROR_CHECK failed at line %d\n", __LINE__); \
            abort();                                                         \
        }                                                                    \
    } while (0)
