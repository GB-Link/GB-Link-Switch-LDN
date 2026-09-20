include("${CMAKE_CURRENT_LIST_DIR}/../../common/SOURCES.cmake")

idf_component_register(
    SRCS ${BRIDGE_SOURCES} ${BRIDGE_TRANSPORT_USB} "xiao_c6_board.c"
    INCLUDE_DIRS "." "${BRIDGE_COMMON}"
    REQUIRES ${BRIDGE_REQUIRES} esp_driver_usb_serial_jtag
)
bridge_pin_wifi_driver(esp32c6 "6cd630f338006fdb7dde1f895eeceff653b1c97fdc8e6b7daeac20759690edbf")
