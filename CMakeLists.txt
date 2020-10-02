cmake_minimum_required(VERSION 3.10)
project(SSH_NATIVE)

set(CMAKE_CXX_STANDARD 17)
set(TARGET ssh_native)

if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else ()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

option(CASE_INSENSITIVE "Case insensitive method names" OFF)

add_library(${TARGET} SHARED
        src/Component.cpp
        src/Component.h
        src/ForwardingServer.cpp
        src/ForwardingServer.h
        src/ForwardingSession.cpp
        src/ForwardingSession.h
        src/SshAddIn.cpp
        src/SshAddIn.h
        src/SshChannel.h
        src/SshSession.h
        src/addin.def
        src/dllmain.cpp
        src/exports.cpp
        src/stdafx.h)

target_compile_definitions(${TARGET} PRIVATE
        UNICODE
        _UNICODE
        OUT_PARAMS
        WIN32_LEAN_AND_MEAN)

if (CASE_INSENSITIVE)
    target_compile_definitions(${TARGET} PRIVATE CASE_INSENSITIVE)
endif ()

target_include_directories(${TARGET} PRIVATE
        include)

target_link_libraries(${TARGET} PRIVATE
        ${CONAN_LIBS})

if (WIN32 AND NOT MSVC)
    message(FATAL_ERROR "Must be compiled with MSVC on Windows")
endif ()

if (WIN32)
    target_compile_definitions(${TARGET} PRIVATE
            _WINDOWS
            _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    target_link_libraries(${TARGET} PRIVATE
            wsock32
            ws2_32)
    target_compile_options(${TARGET} PRIVATE /utf-8)
endif ()

if (UNIX)
    if (TARGET_ARCH STREQUAL "x86")
        set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
    endif ()
endif ()