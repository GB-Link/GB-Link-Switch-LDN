# The bridge's own sources, with pico_link.c (a GB-Link on a UART) swapped for the
# adapter on this chip's pins.
include("${CMAKE_CURRENT_LIST_DIR}/../../../firmware/common/SOURCES.cmake")
list(REMOVE_ITEM BRIDGE_SOURCES "${BRIDGE_COMMON}/pico_link.c")

idf_build_get_property(target IDF_TARGET)
if(target STREQUAL "esp32")
    set(chip_sources ${BRIDGE_TRANSPORT_UART})
    set(chip_requires "")
elseif(target STREQUAL "esp32s3")
    set(chip_sources ${BRIDGE_TRANSPORT_USB})
    set(chip_requires esp_driver_usb_serial_jtag)
elseif(target STREQUAL "esp32c6")
    set(chip_sources ${BRIDGE_TRANSPORT_USB} "${BRIDGE_COMMON}/../ESP32-C6/main/xiao_c6_board.c")
    set(chip_requires esp_driver_usb_serial_jtag)
else()
    message(FATAL_ERROR "The one-board bridge builds for esp32, esp32s3 and esp32c6")
endif()

# Whole archive: a board file replaces the weak board hooks in ldn_probe.c, and nothing
# else references it, so the linker would otherwise keep the weak ones.
idf_component_register(
    SRCS ${BRIDGE_SOURCES} ${chip_sources} "pico_link_local.c"
    INCLUDE_DIRS "." "${BRIDGE_COMMON}"
    REQUIRES ${BRIDGE_REQUIRES} ${chip_requires} rfu_adapter
    WHOLE_ARCHIVE
)

if(target STREQUAL "esp32")
    bridge_pin_wifi_driver(esp32 "018dd68296790dadf4b4e9632f174d6c3a685ef786ec9d7429d3c4c6439f0cf1")
elseif(target STREQUAL "esp32s3")
    bridge_pin_wifi_driver(esp32s3 "265e5c89ac0d2a9444b5b466afee49f631ec549f96774f91a721ab5069c58e20")
else()
    bridge_pin_wifi_driver(esp32c6 "6cd630f338006fdb7dde1f895eeceff653b1c97fdc8e6b7daeac20759690edbf")
endif()
