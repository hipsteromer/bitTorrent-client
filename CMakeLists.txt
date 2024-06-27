# Define the minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Define the project name and version
project(BitTorrentClient VERSION 1.0)

# Define the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find CURL
find_package(CURL REQUIRED)

# Find NCurses
find_package(Curses REQUIRED)

# Define the source directory and source files
set(SRCDIR src)
file(GLOB SOURCES "${SRCDIR}/*.c")

# Add the executable target
add_executable(bittorrent-client ${SOURCES})

# Link libraries
target_link_libraries(bittorrent-client 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    ${CURL_LIBRARIES}
    ${CURSES_LIBRARIES}  # Link NCurses library
)

# Include directories for OpenSSL, CURL, and NCurses
include_directories(
    ${CURL_INCLUDE_DIRS}
    ${CURSES_INCLUDE_DIRS}  # Include NCurses headers
)

# Add compile options
target_compile_options(bittorrent-client PRIVATE -Wall -g)
