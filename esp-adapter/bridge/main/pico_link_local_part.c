/* The adapter on this chip's own pins: pico_link_local.c under names of its own for
   pico_link_select.c. */
#define PICO_LINK_RENAME(name) pico_link_local_##name
#include "pico_link_rename.h"
#include "pico_link_local.c"
