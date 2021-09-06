cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 11)

set(PICO_PLATFORM rp2040)
set(BOARD pico_sdk)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

set(PROJECT picoROM_pio)

project(${PROJECT})

pico_sdk_init()

add_executable(${PROJECT})

target_sources(${PROJECT} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main.c)

pico_generate_pio_header(${PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/eeprom.pio)
pico_generate_pio_header(${PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/eeprom_test.pio)

pico_add_extra_outputs(${PROJECT})

target_link_libraries(${PROJECT} PRIVATE pico_stdlib hardware_pio hardware_dma)
