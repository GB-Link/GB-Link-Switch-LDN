/* A GB-Link on the UART as the adapter: common/pico_link.c under names of its own for
   pico_link_select.c. */
#define PICO_LINK_RENAME(name) pico_link_uart_##name
#include "pico_link_rename.h"
#include "../common/pico_link.c"
