# Define the minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Define the project name and version
project(BitTorrentClient VERSION 1.0)

# Define the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Define the source directory and source files
set(SRCDIR src)
file(GLOB SOURCES "${SRCDIR}/*.c" "${SRCDIR}/*.c")

# Add the executable target
add_executable(bittorrent-client ${SOURCES})

# Link OpenSSL libraries
target_link_libraries(bittorrent-client OpenSSL::SSL OpenSSL::Crypto)

# Add compile options
target_compile_options(bittorrent-client PRIVATE -Wall -g)
