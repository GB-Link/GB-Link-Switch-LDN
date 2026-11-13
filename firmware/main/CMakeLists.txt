# The bridge's own sources, with pico_link.c (a GB-Link on a UART) and the adapter on this
# chip's pins built side by side: pico_link_select.c uses the GB-Link when it finds one on
# the UART, and the chip's own adapter otherwise.
include("${CMAKE_CURRENT_LIST_DIR}/../common/SOURCES.cmake")
list(REMOVE_ITEM BRIDGE_SOURCES "${BRIDGE_COMMON}/pico_link.c")
set(link_select "pico_link_select.c" "pico_link_uart_part.c" "pico_link_local_part.c")

idf_build_get_property(target IDF_TARGET)
if(target STREQUAL "esp32")
    set(chip_sources ${BRIDGE_TRANSPORT_UART})
    set(chip_requires "")
elseif(target STREQUAL "esp32c3" OR target STREQUAL "esp32c5" OR target STREQUAL "esp32s3")
    set(chip_sources ${BRIDGE_TRANSPORT_USB})
    set(chip_requires esp_driver_usb_serial_jtag)
elseif(target STREQUAL "esp32c6")
    set(chip_sources ${BRIDGE_TRANSPORT_USB} "xiao_c6_board.c")
    set(chip_requires esp_driver_usb_serial_jtag)
else()
    message(FATAL_ERROR "The bridge builds for esp32, esp32c3, esp32c5, esp32c6 and esp32s3")
endif()

# Whole archive: a board file replaces the weak board hooks in ldn_probe.c, and nothing
# else references it, so the linker would otherwise keep the weak ones.
idf_component_register(
    SRCS ${BRIDGE_SOURCES} ${link_select} ${chip_sources}
    INCLUDE_DIRS "." "${BRIDGE_COMMON}"
    REQUIRES ${BRIDGE_REQUIRES} ${chip_requires} rfu_adapter
    WHOLE_ARCHIVE
)

if(target STREQUAL "esp32")
    # With the adapter on its pins the original ESP32 has static RAM for three Switches in
    # a room the GBA leads: a group of four.
    target_compile_definitions(${COMPONENT_LIB} PRIVATE PIA_HOST_STATIONS=3)
    bridge_pin_wifi_driver(esp32 "018dd68296790dadf4b4e9632f174d6c3a685ef786ec9d7429d3c4c6439f0cf1")
elseif(target STREQUAL "esp32c3")
    bridge_pin_wifi_driver(esp32c3 "0fcbed322d3254063b2c191bc8b8004e703ffd640296164a6552676180909a0c")
elseif(target STREQUAL "esp32c5")
    bridge_pin_wifi_driver(esp32c5 "4c86fc1d2f40a933af7f672972978eb7c82651d838fd10584e006438d585793b")
elseif(target STREQUAL "esp32c6")
    bridge_pin_wifi_driver(esp32c6 "6cd630f338006fdb7dde1f895eeceff653b1c97fdc8e6b7daeac20759690edbf")
else()
    bridge_pin_wifi_driver(esp32s3 "265e5c89ac0d2a9444b5b466afee49f631ec549f96774f91a721ab5069c58e20")
endif()
