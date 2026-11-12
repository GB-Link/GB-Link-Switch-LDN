#include <stdio.h>

#include "sdkconfig.h"
#include "esp_console.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "gb_link.h"
#include "wa_adapter.h"

/* A GBA wireless adapter on the ESP32's own pins, standing in for a GB-Link in the
   wireless-adapter mode: the bridge board talks to it over the same UART link. */

static const char *const kComstate[] = {
    "waiting for the ID exchange", "ID exchange", "commands", "command words", "reply",
    "reply words", "error reply", "error reply", "waiting for an event", "delivering an event",
    "replaying", "replaying",
};

static void transport_data(const uint8_t *bytes, size_t length, void *ctx)
{
    (void)ctx;
    gb_link_send(GB_LINK_DATA, bytes, length);
}

static void transport_status(uint16_t code, void *ctx)
{
    (void)ctx;
    gb_link_send_status(code);
}

static void start_adapter(uint8_t role)
{
    const wa_config_t config = {
        .pins = {.sc = CONFIG_WA_PIN_SC, .so = CONFIG_WA_PIN_SO, .si = CONFIG_WA_PIN_SI, .sd = CONFIG_WA_PIN_SD},
        .role = role,
        .spin_us = CONFIG_WA_HANDSHAKE_SPIN_US,
    };
    const wa_transport_t transport = {.data = transport_data, .status = transport_status};
    if (!wa_adapter_start(&config, &transport)) printf("WA could not start the adapter task\n");
}

static void on_link(uint8_t channel, const uint8_t *payload, size_t length)
{
    if (channel == GB_LINK_DATA)
    {
        wa_adapter_feed(payload, length);
        return;
    }
    if (channel != GB_LINK_COMMAND || length == 0) return;
    switch (payload[0])
    {
    case 0x00:   /* SetMode: mode, variant. The adapter answers AwaitMode. */
        if (length >= 2 && payload[1] == 0x07) wa_adapter_enter_mode(length >= 3 ? payload[2] : 0);
        break;
    case 0x48:   /* Reboot */
        esp_restart();
        break;
    default:
        break;
    }
}

static void print_words(const char *label, const uint32_t *words, uint8_t count)
{
    printf("WA %s", label);
    for (uint8_t i = 0; i < count; i++) printf(" %08lx", (unsigned long)words[i]);
    printf("\n");
}

static int cmd_status(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    wa_stats_t s;
    wa_adapter_stats(&s);
    uint32_t rx, tx, resync;
    gb_link_stats(&rx, &tx, &resync);
    printf("WA self-test %s: %s\n", s.cal.ok ? "passed" : "FAILED", s.cal.detail);
    printf("WA GBA: %s, last command %02x, %lu commands, %lu words (%lu with a bad bit count, %lu realigned, "
           "%lu with a disputed count of ones)\n",
           s.comstate < sizeof(kComstate) / sizeof(kComstate[0]) ? kComstate[s.comstate] : "?", s.last_cmd,
           (unsigned long)s.commands, (unsigned long)s.words, (unsigned long)s.bad_bits, (unsigned long)s.realigns,
           (unsigned long)s.ones_mismatches);
    printf("WA handshakes: %lu released at once, %lu by interrupt, %lu held\n", (unsigned long)s.released_spin,
           (unsigned long)s.released_irq, (unsigned long)s.held);
    printf("WA events: %lu delivered, %lu failed (last stage %02x), %lu retried, %lu adapter-clocked words\n",
           (unsigned long)s.deliveries, (unsigned long)s.delivery_fails, s.last_fail,
           (unsigned long)s.delivery_retries, (unsigned long)s.master_words);
    printf("WA resets %lu, ID restarts %lu, command restarts %lu, error replies %lu\n", (unsigned long)s.sd_resets,
           (unsigned long)s.login_restarts, (unsigned long)s.command_restarts, (unsigned long)s.err_responses);
    printf("WA network: %lu frames in, %lu out, park high %lu, dropped %lu in %lu out; link %lu/%lu frames, %lu resync\n",
           (unsigned long)s.frames_in, (unsigned long)s.frames_out, (unsigned long)s.parked_high,
           (unsigned long)s.parked_drops, (unsigned long)s.out_drops, (unsigned long)rx, (unsigned long)tx,
           (unsigned long)resync);
    printf("WA lines SC %d SO %d SI %d SD %d\n", s.lines & 1, (s.lines >> 1) & 1, (s.lines >> 2) & 1,
           (s.lines >> 3) & 1);
    return 0;
}

static int cmd_selftest(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    wa_adapter_recalibrate();
    vTaskDelay(pdMS_TO_TICKS(200));
    wa_stats_t s;
    wa_adapter_stats(&s);
    printf("WA self-test %s: %s\n", s.cal.ok ? "passed" : "FAILED", s.cal.detail);
    return 0;
}

static void start_console(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "adapter>";
#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
    esp_console_dev_usb_serial_jtag_config_t device = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&device, &repl_config, &repl));
#else
    esp_console_dev_uart_config_t device = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&device, &repl_config, &repl));
#endif
    const esp_console_cmd_t status = {.command = "status", .help = "Adapter state and counters", .func = cmd_status};
    const esp_console_cmd_t selftest = {.command = "selftest", .help = "Run the SPI self-test again", .func = cmd_selftest};
    ESP_ERROR_CHECK(esp_console_cmd_register(&status));
    ESP_ERROR_CHECK(esp_console_cmd_register(&selftest));
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}

/* Prints the milestones of a bring-up as they happen. */
static void watch_task(void *arg)
{
    (void)arg;
    bool reported_cal = false, reported_rx = false, reported_id = false, reported_found = false;
    uint32_t last_words = 0, last_resets = 0;
    TickType_t last_summary = 0;
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(250));
        if (!wa_adapter_running()) continue;
        wa_stats_t s;
        wa_adapter_stats(&s);
        if (!reported_cal)
        {
            printf("WA self-test %s: %s\n", s.cal.ok ? "passed" : "FAILED", s.cal.detail);
            reported_cal = true;
        }
        if (!reported_rx && s.any_rx)
        {
            print_words("the GBA is clocking:", s.first_words, s.first_count);
            reported_rx = true;
        }
        if (!reported_id && s.nintendo)
        {
            printf("WA the GBA started the ID exchange\n");
            reported_id = true;
        }
        if (!reported_found && s.comstate >= 2 && s.commands > 0)
        {
            printf("WA the GBA found the adapter and sends commands\n");
            reported_found = true;
        }
        if (s.sd_resets != last_resets)
        {
            printf("WA the GBA reset the adapter (%lu)\n", (unsigned long)s.sd_resets);
            last_resets = s.sd_resets;
            reported_id = reported_found = false;
        }
        const TickType_t now = xTaskGetTickCount();
        if (s.words != last_words && now - last_summary >= pdMS_TO_TICKS(10000))
        {
            printf("WA %lu words, %lu commands, %lu events delivered, %lu failed, %lu bad bit counts, %lu realigned\n",
                   (unsigned long)s.words, (unsigned long)s.commands, (unsigned long)s.deliveries,
                   (unsigned long)s.delivery_fails, (unsigned long)s.bad_bits, (unsigned long)s.realigns);
            last_words = s.words;
            last_summary = now;
        }
    }
}

void app_main(void)
{
    printf("WA GBA wireless adapter: SC %d SO %d SI %d SD %d, bridge link TX %d RX %d\n", CONFIG_WA_PIN_SC,
           CONFIG_WA_PIN_SO, CONFIG_WA_PIN_SI, CONFIG_WA_PIN_SD, CONFIG_GB_LINK_TX_GPIO, CONFIG_GB_LINK_RX_GPIO);
    gb_link_start(on_link);
    /* Running from power-up lets the GBA find the adapter before the bridge is up. */
    start_adapter(0);
    xTaskCreate(watch_task, "wa_watch", 3072, NULL, 2, NULL);
    start_console();
}
