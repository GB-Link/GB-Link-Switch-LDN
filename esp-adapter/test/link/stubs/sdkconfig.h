#pragma once
/* What pico_link_local.c needs from a board's configuration (the S3's defaults). */
#define CONFIG_PICO_LINK_TX_GPIO 2
#define CONFIG_PICO_LINK_RX_GPIO 1
#define CONFIG_WA_PIN_SC 5
#define CONFIG_WA_PIN_SO 6
#define CONFIG_WA_PIN_SI 7
#define CONFIG_WA_PIN_SD 8
#define CONFIG_WA_HANDSHAKE_SPIN_US 15
#define CONFIG_FREERTOS_NUMBER_OF_CORES 1
