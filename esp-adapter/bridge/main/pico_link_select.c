#include "pico_link.h"

#include <stdio.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* pico_link.h for a board that may sit in a GB-Link's expansion socket. At power-up a
   GB-Link on the UART pins makes the GB-Link the adapter, as for the two-board bridge, and
   this chip's adapter pins are left alone. Otherwise the adapter runs on this chip's own
   pins. */

#define PICO_LINK_PART_API(part)                                                                \
    void part##_start(void);                                                                    \
    bool part##_set_port(pico_port_t port);                                                     \
    pico_port_t part##_port(void);                                                              \
    void part##_feed_host(const uint8_t *bytes, size_t length);                                 \
    bool part##_poll_host_out(uint8_t *frame, size_t capacity, size_t *length);                 \
    void part##_stop(void);                                                                     \
    bool part##_running(void);                                                                  \
    bool part##_write(const uint8_t *bytes, size_t length);                                     \
    bool part##_send(uint8_t channel, const uint8_t *payload, size_t length);                   \
    bool part##_poll_inbound(uint8_t *frame, size_t capacity, size_t *length);                  \
    void part##_set_mode(void);                                                                 \
    uint32_t part##_mode_sent(void);                                                            \
    bool part##_await_mode(void);                                                               \
    bool part##_wrong_cable(void);                                                              \
    bool part##_gba_active(void);                                                               \
    int64_t part##_gba_search_age_ms(void);                                                     \
    void part##_reset_inbound(void);                                                            \
    void part##_bootsel(void);                                                                  \
    void part##_swap(void);                                                                     \
    bool part##_swapped(void);                                                                  \
    void part##_probe(bool *tx_driven, bool *rx_driven);                                        \
    void part##_stats(uint32_t *rx_frames, uint32_t *tx_frames, uint32_t *rx_bytes,             \
                      uint32_t *resync, uint32_t *dropped);                                     \
    uint8_t part##_first_frame(uint8_t *out, uint8_t capacity);                                 \
    uint8_t part##_frame_log(uint8_t slot, uint8_t *out, uint8_t capacity);                     \
    uint8_t part##_status_log(uint16_t *out, uint8_t capacity);

PICO_LINK_PART_API(pico_link_uart)
PICO_LINK_PART_API(pico_link_local)

/* A GB-Link's UART output (its GP8) rests high; a pin with nothing on it follows the
   pull-down. Watched for up to a second, as the two boards may power up a little apart. */
#define GBLINK_WATCH_US 1000000
#define GBLINK_STEADY_US 50000

static bool s_decided, s_gblink;

/* A UART pin set to one of this chip's adapter pins carries a GBA line, whose resting level
   would pass for a GB-Link's. Such a board has no GB-Link to look for. */
static bool uart_on_adapter_pins(void)
{
    const int adapter[] = {CONFIG_WA_PIN_SC, CONFIG_WA_PIN_SO, CONFIG_WA_PIN_SI, CONFIG_WA_PIN_SD};
    for (size_t i = 0; i < sizeof(adapter) / sizeof(adapter[0]); ++i)
        if (adapter[i] == PICO_LINK_PIN_RX || adapter[i] == PICO_LINK_PIN_TX) return true;
    return false;
}

static bool gblink_on_uart(void)
{
    const gpio_config_t pulled_down = {
        .pin_bit_mask = 1ULL << PICO_LINK_PIN_RX,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&pulled_down);
    const int64_t start = esp_timer_get_time();
    int64_t high_since = -1;
    while (esp_timer_get_time() - start < GBLINK_WATCH_US)
    {
        vTaskDelay(1);
        const int64_t now = esp_timer_get_time();
        if (!gpio_get_level(PICO_LINK_PIN_RX)) high_since = -1;
        else if (high_since < 0) high_since = now;
        else if (now - high_since >= GBLINK_STEADY_US) return true;
    }
    return false;
}

#define PART(name) (s_gblink ? pico_link_uart_##name : pico_link_local_##name)

void pico_link_start(void)
{
    if (!s_decided)
    {
        s_gblink = !uart_on_adapter_pins() && gblink_on_uart();
        s_decided = true;
        if (s_gblink)
            printf("LDN_WA GB-Link on RX GPIO%d TX GPIO%d: the adapter is the GB-Link's\n", PICO_LINK_PIN_RX,
                   PICO_LINK_PIN_TX);
    }
    PART(start)();
}

bool pico_link_set_port(pico_port_t port) { return PART(set_port)(port); }
pico_port_t pico_link_port(void) { return PART(port)(); }
void pico_link_feed_host(const uint8_t *bytes, size_t length) { PART(feed_host)(bytes, length); }
bool pico_link_poll_host_out(uint8_t *frame, size_t capacity, size_t *length)
{
    return PART(poll_host_out)(frame, capacity, length);
}
void pico_link_stop(void) { PART(stop)(); }
bool pico_link_running(void) { return PART(running)(); }
bool pico_link_write(const uint8_t *bytes, size_t length) { return PART(write)(bytes, length); }
bool pico_link_send(uint8_t channel, const uint8_t *payload, size_t length)
{
    return PART(send)(channel, payload, length);
}
bool pico_link_poll_inbound(uint8_t *frame, size_t capacity, size_t *length)
{
    return PART(poll_inbound)(frame, capacity, length);
}
void pico_link_set_mode(void) { PART(set_mode)(); }
uint32_t pico_link_mode_sent(void) { return PART(mode_sent)(); }
bool pico_link_await_mode(void) { return PART(await_mode)(); }
bool pico_link_wrong_cable(void) { return PART(wrong_cable)(); }
bool pico_link_gba_active(void) { return PART(gba_active)(); }
int64_t pico_link_gba_search_age_ms(void) { return PART(gba_search_age_ms)(); }
void pico_link_reset_inbound(void) { PART(reset_inbound)(); }
void pico_link_bootsel(void) { PART(bootsel)(); }
void pico_link_swap(void) { PART(swap)(); }
bool pico_link_swapped(void) { return PART(swapped)(); }
void pico_link_probe(bool *tx_driven, bool *rx_driven) { PART(probe)(tx_driven, rx_driven); }
void pico_link_stats(uint32_t *rx_frames, uint32_t *tx_frames, uint32_t *rx_bytes, uint32_t *resync,
                     uint32_t *dropped)
{
    PART(stats)(rx_frames, tx_frames, rx_bytes, resync, dropped);
}
uint8_t pico_link_first_frame(uint8_t *out, uint8_t capacity) { return PART(first_frame)(out, capacity); }
uint8_t pico_link_frame_log(uint8_t slot, uint8_t *out, uint8_t capacity)
{
    return PART(frame_log)(slot, out, capacity);
}
uint8_t pico_link_status_log(uint16_t *out, uint8_t capacity) { return PART(status_log)(out, capacity); }
