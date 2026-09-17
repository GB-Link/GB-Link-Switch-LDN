#pragma once
#include <stdbool.h>
#include "esp_err.h"

const char *ldn_session_ssid(void);
esp_err_t ldn_session_configure(const char *ssid, const char *bssid, const char *key, unsigned channel);
esp_err_t ldn_session_scan(unsigned channel);
void ldn_session_stop(void);
void ldn_control_target(const unsigned char host[6]);

/* Signal strength of the newest LDN frame from the Switch, in dBm (0 = none seen).
   A radio wired to the wrong antenna path still links up close range and drops
   frames, which is indistinguishable from a busy CPU without this. */
int8_t ldn_probe_last_rssi(void);

/* Frames seen and their dBm since the last call, which this resets. */
void ldn_probe_rssi_window(int *count, int *average, int *low, int *high);

/* Boards with a switched antenna implement this; true if the board took it.
   external: false for the module's own antenna, true for a connector. */
bool bridge_board_antenna(bool external);
