# karty

## What is karty?

- Provide a description of the use case for this repo...

## Code Style Guidelines

- Please use the `.clang-format` style. (You can automatically apply the style by selecting it from the context menu.)

<img src="img\format_document.png" alt="screenshot of context menu" width="250">

- Use the [`esp_log_alias`](https://github.com/Giddy-Up224/esp_log_alias) library (an alias for `ESP_LOGI`, `ESP_LOGD`, etc.). rather than `Serial.println()` etc. (see [official documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/log.html) for instructions on use.) 
    
    **Note:** You MUST add this line to your `platformio.ini` file if you want `debug` and `info` log levels: `build_flags = -DCORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_DEBUG` 

    Info Source: [PlatformIO discussion](https://community.platformio.org/t/default-log-level-for-esp32-arduino-changed/3178/5)

## Links and Resources

- Provide links and References to material that is relevant to the project...

## Credits

- Give the right people/sources the credit for contributions etc.