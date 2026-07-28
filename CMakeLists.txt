cmake_minimum_required(VERSION 3.20)
project(PcPosNetworkManager VERSION 1.0.0 LANGUAGES C RC)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(PcPosNetworkManager WIN32
    src/main.c
    src/network.c
    src/config.c
    src/utils.c
    src/device.c
    src/license.c
    src/network_report.c
    resources/app.rc
)

target_include_directories(PcPosNetworkManager PRIVATE src)
target_compile_definitions(PcPosNetworkManager PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WIN32_WINNT=0x0601
)

target_link_libraries(PcPosNetworkManager PRIVATE
    iphlpapi
    ws2_32
    comctl32
    shell32
    advapi32
    user32
    gdi32
    winhttp
)

if(MSVC)
    # Always enable high warning level and UTF-8 for MSVC.
    # Only add /O2 when building Release to avoid mixing Debug runtime-check flags
    # (like /RTC1) with optimizations which cause D8016 errors.
    target_compile_options(PcPosNetworkManager PRIVATE /W4 /utf-8 $<$<CONFIG:Release>:/O2>)
else()
    target_compile_options(PcPosNetworkManager PRIVATE
        -Wall -Wextra -Wpedantic -O2 -finput-charset=UTF-8
    )
    target_link_options(PcPosNetworkManager PRIVATE -municode -mwindows -static-libgcc)
endif()

set_target_properties(PcPosNetworkManager PROPERTIES
    OUTPUT_NAME "PcPosNetworkManager"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/release"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release"
)
