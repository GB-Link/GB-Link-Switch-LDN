include("${CMAKE_CURRENT_LIST_DIR}/../../bridge-common/SOURCES.cmake")

idf_component_register(
    SRCS ${BRIDGE_SOURCES} "xiao_c6_board.c"
    INCLUDE_DIRS "." "${BRIDGE_COMMON}"
    REQUIRES ${BRIDGE_REQUIRES}
)
if(NOT CONFIG_IDF_TARGET_ESP32C6)
    message(FATAL_ERROR "This project is for ESP32-C6 only")
endif()
# The private WPA callback table and key-install entry points this port drives are
# private ABI; a different driver binary needs re-verification on hardware.
file(SHA256 "$ENV{IDF_PATH}/components/esp_wifi/lib/esp32c6/libnet80211.a" c6_driver_sha256)
if(NOT c6_driver_sha256 STREQUAL "6cd630f338006fdb7dde1f895eeceff653b1c97fdc8e6b7daeac20759690edbf")
    message(FATAL_ERROR "C6 WPA callbacks and key APIs require the pinned v6.1 C6 driver binary")
endif()
