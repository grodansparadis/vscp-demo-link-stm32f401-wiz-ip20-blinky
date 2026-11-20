cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME firmware)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/flash_storage.c
    wiznet-ip20.c
    ${CMAKE_CURRENT_SOURCE_DIR}/callbacks-link.c
    ${CMAKE_CURRENT_SOURCE_DIR}/callbacks-frm2.c
    #${CMAKE_CURRENT_SOURCE_DIR}/callbacks-binary.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/vscp-firmware/src/vscp-fifo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/vscp-firmware/src/vscp-aes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/vscp-firmware/src/vscp-crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/vscp-firmware/src/vscp-firmware-helper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/vscp-firmware/src/vscp-link-protocol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/vscp-firmware/src/vscp-firmware-level2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/vscp-firmware/src/vscp-binary-protocol.c
)

# Build-number increment: runs before every build, generates build_number.h
add_custom_target(increment_build_number ALL
    COMMAND ${CMAKE_COMMAND}
        -DBUILD_OUTDIR=${CMAKE_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/increment_build.cmake
    COMMENT "Incrementing build number"
    VERBATIM
)
add_dependencies(${CMAKE_PROJECT_NAME} increment_build_number)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/vscp-firmware/src/
)

# Transport (see blinky.h): TCP_SERVER (default), TCP_CLIENT, UDP, MULTICAST or MQTT
set(BLINKY_MODE "TCP_SERVER" CACHE STRING "Blinky transport")
set_property(CACHE BLINKY_MODE PROPERTY STRINGS TCP_SERVER TCP_CLIENT UDP MULTICAST MQTT)
if(NOT BLINKY_MODE MATCHES "^(TCP_SERVER|TCP_CLIENT|UDP|MULTICAST|MQTT)$")
    message(FATAL_ERROR "BLINKY_MODE must be TCP_SERVER, TCP_CLIENT, UDP, MULTICAST or MQTT")
endif()
message(STATUS "Blinky transport: ${BLINKY_MODE}")

# Remote host (TCP client server, UDP peer, MQTT broker), e.g. -DBLINKY_REMOTE_HOST=192.168.1.9
set(BLINKY_REMOTE_HOST "" CACHE STRING "Remote host IP address (empty: the default in wiznet-ip20.h)")
set(BLINKY_REMOTE_PORT "" CACHE STRING "Remote port (empty: the default for the transport)")

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    BLINKY_MODE_${BLINKY_MODE}
    $<$<BOOL:${BLINKY_REMOTE_HOST}>:BLINKY_REMOTE_HOST="${BLINKY_REMOTE_HOST}">
    $<$<BOOL:${BLINKY_REMOTE_PORT}>:BLINKY_REMOTE_PORT=${BLINKY_REMOTE_PORT}>
)

# MQTT sends VSCP JSON events: cJSON (same pin as the vscp-firmware tests).
# Use -DFETCHCONTENT_SOURCE_DIR_CJSON=<path> for a local checkout.
if(BLINKY_MODE STREQUAL "MQTT")
    include(FetchContent)
    set(_cjson_options)
    if(NOT CMAKE_VERSION VERSION_LESS "3.18")
        list(APPEND _cjson_options SOURCE_SUBDIR "__sources_only__")
    endif()
    if(NOT CMAKE_VERSION VERSION_LESS "3.24")
        list(APPEND _cjson_options DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    endif()
    FetchContent_Declare(
        cjson
        URL https://codeload.github.com/DaveGamble/cJSON/tar.gz/b2890c8d76bbb64e710585ebc0a917196b9c67e7
        URL_HASH SHA256=c3866048d822428ab289d20842c75a39c92c26e967fed2f0706f6f15e6894082
        ${_cjson_options}
    )
    if(CMAKE_VERSION VERSION_LESS "3.18")
        FetchContent_GetProperties(cjson)
        if(NOT cjson_POPULATED)
            FetchContent_Populate(cjson)
        endif()
    else()
        FetchContent_MakeAvailable(cjson)
    endif()
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${cjson_SOURCE_DIR}/cJSON.c)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${cjson_SOURCE_DIR})
endif()

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Optimize for size and enable LTO for non-Debug builds
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    #target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Os -flto -ffunction-sections -fdata-sections)
    #target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -Wl,--gc-sections -flto)
else()
    # -Og: optimize for debuggability; -g3: full debug info; no LTO so the
    # debugger can resolve symbols and breakpoints correctly.
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Og -g3 -ffunction-sections -fdata-sections)
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -Wl,--gc-sections)
endif()

# Post-build: generate .hex and .bin from .elf
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${CMAKE_PROJECT_NAME}> $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating HEX and BIN from ELF"
)
