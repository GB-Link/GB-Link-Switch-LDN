# The bridge firmware itself: LDN radio, the Pia/PiaLink session with the Switch,
# the trade shim and the UART link to the Pico. Chip-independent -- each board's
# project under firmware/ supplies only its target, pinned Wi-Fi driver hash and
# sdkconfig, so a fix lands on every board at once.
set(BRIDGE_COMMON "${CMAKE_CURRENT_LIST_DIR}")
set(BRIDGE_SOURCES
    "${BRIDGE_COMMON}/ldn_probe.c"
    "${BRIDGE_COMMON}/ldn_control.c"
    "${BRIDGE_COMMON}/ldn_udp.c"
    "${BRIDGE_COMMON}/ldn_wire.c"
    "${BRIDGE_COMMON}/ldn_keys.c"
    "${BRIDGE_COMMON}/usb_transport.c"
    "${BRIDGE_COMMON}/pico_link.c"
    "${BRIDGE_COMMON}/trade_shim.c"
    "${BRIDGE_COMMON}/pia_crypto.c"
    "${BRIDGE_COMMON}/pia_test.c"
    "${BRIDGE_COMMON}/pia_reliable.c"
    "${BRIDGE_COMMON}/pia_conn.c"
    "${BRIDGE_COMMON}/pia_link.c"
    "${BRIDGE_COMMON}/pia_bridge.c")
set(BRIDGE_REQUIRES
    esp_event esp_netif esp_wifi esp_timer nvs_flash wpa_supplicant esp_hw_support
    spi_flash esp_driver_usb_serial_jtag lwip esp_driver_gpio esp_driver_uart mbedtls zstd)
