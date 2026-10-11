#include "gb_link.h"

#include "sdkconfig.h"
#include "driver/uart.h"
#include "soc/uart_reg.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define LINK_UART UART_NUM_1
#define LINK_BAUD 921600
#define MAX_PAYLOAD 128

static gb_link_handler_t s_handler;
static SemaphoreHandle_t s_tx_lock;
static uint32_t s_rx_frames, s_tx_frames, s_resync;

static void rx_task(void *arg)
{
    (void)arg;
    enum { SYNC0, SYNC1, CHANNEL, LEN_LO, LEN_HI, PAYLOAD } state = SYNC0;
    uint8_t channel = 0;
    uint16_t length = 0, seen = 0;
    uint8_t payload[MAX_PAYLOAD];
    uint8_t buffer[256];
    for (;;)
    {
        const int n = uart_read_bytes(LINK_UART, buffer, sizeof(buffer), pdMS_TO_TICKS(20));
        for (int i = 0; i < n; i++)
        {
            const uint8_t b = buffer[i];
            switch (state)
            {
            case SYNC0:
                if (b == 0x47) state = SYNC1;
                else s_resync++;
                break;
            case SYNC1:
                if (b == 0x42) state = CHANNEL;
                else
                {
                    s_resync++;
                    state = b == 0x47 ? SYNC1 : SYNC0;
                }
                break;
            case CHANNEL:
                channel = b;
                state = LEN_LO;
                break;
            case LEN_LO:
                length = b;
                state = LEN_HI;
                break;
            case LEN_HI:
                length |= (uint16_t)b << 8;
                seen = 0;
                if (length > MAX_PAYLOAD)
                {
                    s_resync++;
                    state = SYNC0;
                }
                else if (length == 0)
                {
                    s_rx_frames++;
                    s_handler(channel, payload, 0);
                    state = SYNC0;
                }
                else
                    state = PAYLOAD;
                break;
            case PAYLOAD:
                payload[seen++] = b;
                if (seen == length)
                {
                    s_rx_frames++;
                    s_handler(channel, payload, length);
                    state = SYNC0;
                }
                break;
            }
        }
    }
}

void gb_link_start(gb_link_handler_t handler)
{
    s_handler = handler;
    s_tx_lock = xSemaphoreCreateMutex();
    const uart_config_t config = {
        .baud_rate = LINK_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_driver_install(LINK_UART, 4096, 4096, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(LINK_UART, &config));
    ESP_ERROR_CHECK(uart_set_pin(LINK_UART, CONFIG_GB_LINK_TX_GPIO, CONFIG_GB_LINK_RX_GPIO,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    /* The bridge's frames arrive in bursts at 921600: take them off the FIFO early. */
    const uart_intr_config_t interrupts = {
        .intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M,
        .rxfifo_full_thresh = 32,
        .rx_timeout_thresh = 2,
    };
    ESP_ERROR_CHECK(uart_intr_config(LINK_UART, &interrupts));
    xTaskCreate(rx_task, "gb_link_rx", 4096, NULL, 12, NULL);
}

void gb_link_send(uint8_t channel, const uint8_t *payload, size_t length)
{
    if (length > MAX_PAYLOAD || !s_tx_lock) return;
    const uint8_t header[5] = {0x47, 0x42, channel, (uint8_t)length, (uint8_t)(length >> 8)};
    xSemaphoreTake(s_tx_lock, portMAX_DELAY);
    uart_write_bytes(LINK_UART, header, sizeof(header));
    if (length) uart_write_bytes(LINK_UART, payload, length);
    s_tx_frames++;
    xSemaphoreGive(s_tx_lock);
}

void gb_link_send_status(uint16_t code)
{
    const uint8_t payload[2] = {(uint8_t)code, (uint8_t)(code >> 8)};
    gb_link_send(GB_LINK_STATUS, payload, sizeof(payload));
}

void gb_link_stats(uint32_t *rx_frames, uint32_t *tx_frames, uint32_t *resync)
{
    *rx_frames = s_rx_frames;
    *tx_frames = s_tx_frames;
    *resync = s_resync;
}
