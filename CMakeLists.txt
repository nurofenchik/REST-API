cmake_minimum_required(VERSION 3.20)
project(RestAPI VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS Homebrew paths
if(APPLE)
    set(CMAKE_PREFIX_PATH "/opt/homebrew")
    include_directories("/opt/homebrew/include")
    link_directories("/opt/homebrew/lib")
endif()

# Install nlohmann/json and jwt-cpp via Homebrew
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)

# SQLite
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# OpenSSL
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${SQLITE3_LIBRARIES}
    nlohmann_json::nlohmann_json
    OpenSSL::SSL 
    OpenSSL::Crypto
    pthread
)

# Include directories for libraries
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SQLITE3_INCLUDE_DIRS}
)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${SQLITE3_CFLAGS_OTHER})
