#pragma once
/* What pico_link.c needs from a board's configuration, for building it on a host
   (see tools/pico_link_test.c). */
#define CONFIG_PICO_LINK_TX_GPIO 17
#define CONFIG_PICO_LINK_RX_GPIO 16
#define CONFIG_FREERTOS_NUMBER_OF_CORES 1
