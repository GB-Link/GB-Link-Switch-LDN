include("${CMAKE_CURRENT_LIST_DIR}/../../common/SOURCES.cmake")

idf_component_register(
    SRCS ${BRIDGE_SOURCES} ${BRIDGE_TRANSPORT_UART}
    INCLUDE_DIRS "." "${BRIDGE_COMMON}"
    REQUIRES ${BRIDGE_REQUIRES}
)
bridge_pin_wifi_driver(esp32 "018dd68296790dadf4b4e9632f174d6c3a685ef786ec9d7429d3c4c6439f0cf1")
