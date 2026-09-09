#include "esp32_transport.h"
#include "sdkconfig.h"
#include "driver/uart.h"
#include "driver/uart_vfs.h"
#include "freertos/FreeRTOS.h"
#include "esp_err.h"

static uint32_t dropped;

void esp32_transport_init(void)
{
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 32768, 4096, 0, NULL, 0));
    uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);
}

int esp32_transport_read(void *buffer, size_t length)
{
    return uart_read_bytes(CONFIG_ESP_CONSOLE_UART_NUM, buffer, length, 0);
}

void esp32_transport_write(const void *buffer, size_t length)
{
    /* Blocks only while the 4 KiB TX ring is full; the UDP poll bounds bursts to two packets. */
    int written = uart_write_bytes(CONFIG_ESP_CONSOLE_UART_NUM, buffer, length);
    if (written != (int)length) ++dropped;
}

uint32_t esp32_transport_dropped(void) { return dropped; }

void esp32_transport_set_baud(int baud)
{
    /* The acknowledgement must finish at the old rate before either side switches. */
    uart_wait_tx_done(CONFIG_ESP_CONSOLE_UART_NUM, pdMS_TO_TICKS(1000));
    uart_set_baudrate(CONFIG_ESP_CONSOLE_UART_NUM, baud);
}
