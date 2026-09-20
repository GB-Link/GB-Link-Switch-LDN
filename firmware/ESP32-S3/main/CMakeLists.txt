include("${CMAKE_CURRENT_LIST_DIR}/../../common/SOURCES.cmake")

idf_component_register(
    SRCS ${BRIDGE_SOURCES} ${BRIDGE_TRANSPORT_USB}
    INCLUDE_DIRS "." "${BRIDGE_COMMON}"
    REQUIRES ${BRIDGE_REQUIRES} esp_driver_usb_serial_jtag
)
bridge_pin_wifi_driver(esp32s3 "265e5c89ac0d2a9444b5b466afee49f631ec549f96774f91a721ab5069c58e20")
