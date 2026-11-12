/* A GB-Link on the UART as the adapter: firmware/common/pico_link.c, the two-board bridge's
   link, under names of its own for pico_link_select.c. */
#define PICO_LINK_RENAME(name) pico_link_uart_##name
#include "pico_link_rename.h"
#include "../../../firmware/common/pico_link.c"
