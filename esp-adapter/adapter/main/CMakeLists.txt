idf_component_register(
    SRCS "main.c" "gb_link.c"
    INCLUDE_DIRS "."
    REQUIRES rfu_adapter console esp_driver_uart
)
