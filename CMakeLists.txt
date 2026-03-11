cmake_minimum_required(VERSION 3.28)

project(Encryptie VERSION 1.0 LANGUAGES CXX C)


add_library(picosha2 INTERFACE)
target_sources(picosha2 INTERFACE
    FILE_SET HEADERS
    BASE_DIRS "${PROJECT_SOURCE_DIR}/libs/PicoSHA2"
    FILES "${PROJECT_SOURCE_DIR}/libs/PicoSHA2/picosha2.h"
)

add_library(tiny-aes STATIC "${PROJECT_SOURCE_DIR}/libs/tiny-AES-c/aes.c")

target_compile_features(tiny-aes PRIVATE c_std_99)

target_sources(tiny-aes PUBLIC
    FILE_SET HEADERS
    BASE_DIRS "${PROJECT_SOURCE_DIR}/libs/tiny-AES-c"
    FILES 
        "${PROJECT_SOURCE_DIR}/libs/tiny-AES-c/aes.h"
        "${PROJECT_SOURCE_DIR}/libs/tiny-AES-c/aes.hpp"
)

add_executable(Encryptie)
target_sources(Encryptie PRIVATE "src/main.cpp" "src/TerminalPassword.cpp")
target_compile_features(Encryptie PRIVATE cxx_std_23)

target_include_directories(Encryptie PRIVATE "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(Encryptie PRIVATE picosha2 tiny-aes)