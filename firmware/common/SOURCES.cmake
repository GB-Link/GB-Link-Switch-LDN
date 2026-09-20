# The bridge firmware itself: the LDN radio, the Pia session with the Switch, the link
# shim, the console protocol and the UART link to the GB-Link. Chip-independent: each
# project under firmware/ supplies its target, its console transport, its pinned Wi-Fi
# driver hash and its sdkconfig, so a fix lands on every chip at once.
set(BRIDGE_COMMON "${CMAKE_CURRENT_LIST_DIR}")
set(BRIDGE_SOURCES
    "${BRIDGE_COMMON}/ldn_probe.c"
    "${BRIDGE_COMMON}/ldn_control.c"
    "${BRIDGE_COMMON}/ldn_udp.c"
    "${BRIDGE_COMMON}/ldn_wire.c"
    "${BRIDGE_COMMON}/ldn_keys.c"
    "${BRIDGE_COMMON}/pico_link.c"
    "${BRIDGE_COMMON}/trade_shim.c"
    "${BRIDGE_COMMON}/pia_crypto.c"
    "${BRIDGE_COMMON}/pia_test.c"
    "${BRIDGE_COMMON}/pia_reliable.c"
    "${BRIDGE_COMMON}/pia_conn.c"
    "${BRIDGE_COMMON}/pia_link.c"
    "${BRIDGE_COMMON}/pia_bridge.c")
# Console transports: native USB Serial/JTAG, or the console UART behind a USB bridge.
set(BRIDGE_TRANSPORT_USB "${BRIDGE_COMMON}/transport_usb_serial_jtag.c")
set(BRIDGE_TRANSPORT_UART "${BRIDGE_COMMON}/transport_uart.c")
set(BRIDGE_REQUIRES
    esp_event esp_netif esp_wifi esp_timer nvs_flash wpa_supplicant esp_hw_support
    spi_flash lwip esp_driver_gpio esp_driver_uart mbedtls zstd)

# The private WPA callback table and key-install entry points are private ABI, so each
# chip's Wi-Fi driver binary is pinned; a different one needs re-verifying on hardware.
function(bridge_pin_wifi_driver chip sha256)
    file(SHA256 "$ENV{IDF_PATH}/components/esp_wifi/lib/${chip}/libnet80211.a" actual)
    if(NOT actual STREQUAL "${sha256}")
        message(FATAL_ERROR "The ${chip} join needs the pinned ESP-IDF v6.1 Wi-Fi driver binary")
    endif()
endfunction()
