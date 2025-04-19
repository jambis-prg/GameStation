# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.22)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
set(PICOTOOL_FETCH_FROM_GIT_PATH PICO_SDK_PATH)

project(GameStation)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(GameStation
    src/main.cpp src/lcd.cpp src/input.cpp src/renderer.cpp src/timer.cpp src/audio_manager.cpp
)

target_include_directories(GameStation PRIVATE src)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(GameStation pico_stdlib hardware_adc hardware_pwm pico_multicore hardware_sync)

pico_enable_stdio_usb(GameStation 1)
pico_enable_stdio_uart(GameStation 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(GameStation)