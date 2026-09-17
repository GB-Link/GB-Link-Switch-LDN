#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_netif.h"

void ldn_udp_init(esp_netif_t *netif, const uint8_t host[6]);
bool ldn_udp_command(const char *line, bool connected);
void ldn_udp_poll(bool connected);
void ldn_udp_stop(void);

/* Binary datagram path, used once the session layer runs on-device rather than
   over the text protocol. With a handler set, arrivals are delivered instead of printed. */
bool ldn_udp_send(const char *ip, const uint8_t *data, size_t length);
/* Keep the session alive. The 10 s watchdog exists to drop the radio when the host
   stops pinging; with the session layer on-device the bridge is the one keeping it. */
void ldn_udp_heartbeat(void);
void ldn_udp_set_handler(void (*handler)(const char *ip, const uint8_t *data, size_t length));
