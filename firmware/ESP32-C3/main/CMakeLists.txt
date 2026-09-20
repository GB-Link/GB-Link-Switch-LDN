include("${CMAKE_CURRENT_LIST_DIR}/../../common/SOURCES.cmake")

idf_component_register(
    SRCS ${BRIDGE_SOURCES} ${BRIDGE_TRANSPORT_USB}
    INCLUDE_DIRS "." "${BRIDGE_COMMON}"
    REQUIRES ${BRIDGE_REQUIRES} esp_driver_usb_serial_jtag
)
bridge_pin_wifi_driver(esp32c3 "0fcbed322d3254063b2c191bc8b8004e703ffd640296164a6552676180909a0c")
