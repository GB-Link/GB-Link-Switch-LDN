idf_component_register(
    SRCS "wa_adapter.cpp" "wa_port_esp.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_driver_spi esp_driver_gpio esp_driver_pcnt esp_hal_gpspi esp_hal_gpio
                  esp_timer esp_hw_support esp_rom freertos
)
# rfuProtocol.hpp is vendored unchanged from the GB-Link firmware. Its volatile
# counters and one read guarded by a length GCC cannot follow warn here.
target_compile_options(${COMPONENT_LIB} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile -Wno-maybe-uninitialized>)
