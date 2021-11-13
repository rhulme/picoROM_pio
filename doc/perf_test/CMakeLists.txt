cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 11)

set(PICO_PLATFORM rp2040)
set(BOARD pico_sdk)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

set(PROJECT picoROM_pio_perf_test)

project(${PROJECT})

pico_sdk_init()

set(ROMC picoROM_CTest)
add_executable(${ROMC})

target_sources(${ROMC} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
target_compile_definitions(${ROMC} PUBLIC USE_PIO=0)

pico_generate_pio_header(${ROMC} ${CMAKE_CURRENT_SOURCE_DIR}/eeprom.pio)
pico_generate_pio_header(${ROMC} ${CMAKE_CURRENT_SOURCE_DIR}/eeprom_test.pio)

pico_add_extra_outputs(${ROMC})

target_link_libraries(${ROMC} PRIVATE pico_stdlib hardware_pio hardware_dma)



set(ROMPIO picoROM_PIOTest)
add_executable(${ROMPIO})

target_sources(${ROMPIO} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
target_compile_definitions(${ROMPIO} PUBLIC USE_PIO=1)

pico_generate_pio_header(${ROMPIO} ${CMAKE_CURRENT_SOURCE_DIR}/eeprom.pio)
pico_generate_pio_header(${ROMPIO} ${CMAKE_CURRENT_SOURCE_DIR}/eeprom_test.pio)

pico_add_extra_outputs(${ROMPIO})

target_link_libraries(${ROMPIO} PRIVATE pico_stdlib hardware_pio hardware_dma)
