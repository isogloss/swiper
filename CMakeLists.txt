cmake_minimum_required(VERSION 3.15)

project(Swiper VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific settings
if(NOT WIN32)
    message(FATAL_ERROR "This project is Windows-only")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Global compiler flags
if(MSVC)
    add_compile_options(/W4)
    # Enable multi-processor compilation
    add_compile_options(/MP)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Add subdirectories
add_subdirectory(src/main)
add_subdirectory(src/capture_dll)

# Print build configuration
message(STATUS "")
message(STATUS "Swiper Build Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "")
