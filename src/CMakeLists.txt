cmake_minimum_required(VERSION 3.10)

# Project name and language
project(bitTorrent_client C)

# Set the C standard to C99
set(CMAKE_C_STANDARD 99)

# Find OpenSSL package
find_package(OpenSSL REQUIRED)

# Specify the directory for the executable
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Add all source files to the executable
add_executable(bittorrent main.c decode.c info.c bencode.c sha1/sha1.c)

# Include the directory containing sha1.h
target_include_directories(bittorrent PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sha1)

# Link OpenSSL libraries to the executable
target_link_libraries(bittorrent OpenSSL::Crypto)
