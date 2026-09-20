/* Seeed XIAO ESP32C6 board setup.
 *
 * The module's antenna path runs through an RF switch that is held off until
 * GPIO3 is driven low, and GPIO14 then selects which antenna it connects: low
 * for the onboard ceramic one, high for a U.FL external. Left alone, both pins
 * float and the radio transmits into a disconnected switch -- it associates
 * with strong nearby APs and nothing else, which looks like a range problem
 * rather than a configuration one. Set before esp_wifi_init.
 */
#include "driver/gpio.h"
#include "ldn_session.h"

#define RF_SWITCH_ENABLE_PIN 3   /* active low */
#define ANTENNA_SELECT_PIN   14  /* low: onboard, high: external U.FL */

bool bridge_board_antenna(bool external)
{
    gpio_set_level(ANTENNA_SELECT_PIN, external ? 1 : 0);
    return true;
}

void bridge_board_init(void)
{
    gpio_config_t pins = {
        .pin_bit_mask = (1ULL << RF_SWITCH_ENABLE_PIN) | (1ULL << ANTENNA_SELECT_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&pins);
    gpio_set_level(RF_SWITCH_ENABLE_PIN, 0);
    gpio_set_level(ANTENNA_SELECT_PIN, 0);
}
