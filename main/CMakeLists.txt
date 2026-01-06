idf_component_register(
    SRCS 
        "main.c"
        "Dri/humidifier.c"
        "Dri/voice_ctrl.c"
        "Dri/Dri_ble.c"
        "App/App_ble.c"
        "Dri/Dri_key.c"
        "App/App_key.c"
        "App/App_wifi.c"
        "Dri/Dri_WS2812.c"
        "App/App_led.c"

    INCLUDE_DIRS 
        "."
        "Dri"
        "App"
    
)