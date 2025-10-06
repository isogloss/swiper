cmake_minimum_required(VERSION 3.15)

# Capture DLL target
add_library(capture_dll SHARED
    dllmain.cpp
    hooking.cpp
)

# Rename output to capture.dll
set_target_properties(capture_dll PROPERTIES
    OUTPUT_NAME "capture"
    PREFIX ""
)

# Link against DirectX and Windows libraries
target_link_libraries(capture_dll
    PRIVATE
        d3d11
        d3d9
        dxgi
        user32
        kernel32
)

# Include directories
target_include_directories(capture_dll
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../main  # For ipc.h
)

# Define DLL export/import macro
target_compile_definitions(capture_dll
    PRIVATE
        CAPTURE_DLL_EXPORTS
)

# TODO: Link against MinHook when it's added to dependencies
# target_include_directories(capture_dll PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/minhook/include)
# target_link_libraries(capture_dll PRIVATE minhook)
