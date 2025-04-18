idf_component_register(
    SRCS "cocoplus_main.c" "../components/wifi_utils/wifi_utils.c" "../components/mqtt_client/my_mqtt_client.c" "../components/sensors/sensors.c" "../tasks/drive.c" "../tasks/publish.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES nvs_flash esp_event esp_wifi mqtt spi_flash driver esp_timer
)
