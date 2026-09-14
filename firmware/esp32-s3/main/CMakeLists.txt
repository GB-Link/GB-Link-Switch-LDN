idf_component_register(
    SRCS "ldn_probe.c" "ldn_control.c" "ldn_udp.c" "ldn_wire.c" "s3_transport.c"
         "pico_link.c" "trade_shim.c" "pia_crypto.c" "pia_test.c" "ldn_keys.c" "pia_reliable.c" "pia_conn.c" "pia_link.c" "pia_bridge.c"
    INCLUDE_DIRS "."
    REQUIRES esp_event esp_netif esp_wifi esp_timer nvs_flash wpa_supplicant esp_hw_support spi_flash esp_driver_usb_serial_jtag lwip esp_driver_gpio esp_driver_uart mbedtls zstd
)
if(NOT CONFIG_IDF_TARGET_ESP32S3)
    message(FATAL_ERROR "This project is for ESP32-S3 only")
endif()
# The pinned v6.1 S3 driver this port was built and bench-tested against; the WPA callback table and
# key-install entry points are private ABI, so a different binary needs re-verification.
file(SHA256 "$ENV{IDF_PATH}/components/esp_wifi/lib/esp32s3/libnet80211.a" s3_driver_sha256)
if(NOT s3_driver_sha256 STREQUAL "265e5c89ac0d2a9444b5b466afee49f631ec549f96774f91a721ab5069c58e20")
    message(FATAL_ERROR "S3 WPA callbacks and key APIs require the pinned v6.1 S3 driver binary")
endif()

