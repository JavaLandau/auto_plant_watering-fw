cmake_minimum_required(VERSION 3.5)

set(CMAKE_C_COMPILER sdcc)
set(CMAKE_SYSTEM_NAME Generic)

project(autoplant_watering LANGUAGES C)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(HEADERS "drivers"
            "drivers/ch552"
            "drivers/ch552/inc"
            "common"
)

set(SOURCES "application/main.c"
            "drivers/ch552/src/ch552_sys.c"
            "drivers/ch552/src/ch552_gpio.c"
            "drivers/ch552/src/ch552_timer.c"
            "drivers/ch552/src/ch552_uart.c"
            "drivers/ch552/src/ch552_adc.c"
)

include_directories(${HEADERS})

set(CMAKE_C_FLAGS "-V -mmcs51 --stack-auto --model-small --xram-size 0x0400 --xram-loc 0x0000 --code-size 0x3800")

add_executable(autoplant_watering ${SOURCES})

add_custom_command(TARGET autoplant_watering POST_BUILD 
                   COMMAND makebin.exe -p \"${CMAKE_CURRENT_BINARY_DIR}/autoplant_watering.ihx\" \"${CMAKE_CURRENT_BINARY_DIR}/autoplant_watering.bin\")

add_custom_target(flash COMMAND python.exe -m ch55xtool -f \"${CMAKE_CURRENT_BINARY_DIR}/autoplant_watering.bin\" 
                        --verify_flash \"${CMAKE_CURRENT_BINARY_DIR}/autoplant_watering.bin\" -v -r)