cmake_minimum_required(VERSION 3.15)

# Define the DLL target
add_library(capture_dll SHARED
    dllmain.cpp
    hooking.cpp
)

# Add include directories
target_include_directories(capture_dll PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/main  # For ipc.h
)

# Link Windows and DirectX libraries
target_link_libraries(capture_dll PRIVATE
    kernel32
    d3d11
    dxgi
)

# Conditionally add MinHook if found
if(MINHOOK_FOUND)
    target_include_directories(capture_dll PRIVATE ${MINHOOK_INCLUDE_DIR})
    target_link_libraries(capture_dll PRIVATE ${MINHOOK_LIBRARY})
    target_compile_definitions(capture_dll PRIVATE MINHOOK_AVAILABLE)
    message(STATUS "capture_dll: MinHook support enabled")
else()
    message(STATUS "capture_dll: Building without MinHook support")
endif()

# Set output name
set_target_properties(capture_dll PROPERTIES
    OUTPUT_NAME "capture"
    PREFIX ""  # Remove 'lib' prefix on Unix-like systems
)

# Windows-specific settings
if(MSVC)
    target_compile_options(capture_dll PRIVATE /W4)
endif()
