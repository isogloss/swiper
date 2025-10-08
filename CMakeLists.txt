cmake_minimum_required(VERSION 3.15)
project(Swiper VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories for all configurations
# For multi-configuration generators (like Visual Studio), this ensures
# that outputs go directly to build/Release, build/Debug, etc.
# For single-configuration generators (like Makefiles), use CMAKE_BUILD_TYPE
if(CMAKE_CONFIGURATION_TYPES)
    # Multi-configuration generator (Visual Studio, Xcode)
    foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/${CONFIG_TYPE}")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/lib")
    endforeach()
else()
    # Single-configuration generator (Makefiles, Ninja)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

# Windows specific settings
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# Try to find Dear ImGui in dependencies directory
set(IMGUI_ROOT "${CMAKE_SOURCE_DIR}/dependencies/imgui")
if(EXISTS "${IMGUI_ROOT}/imgui.h")
    message(STATUS "Found Dear ImGui at ${IMGUI_ROOT}")
    set(IMGUI_FOUND TRUE)
    set(IMGUI_INCLUDE_DIR "${IMGUI_ROOT}")
    set(IMGUI_SOURCES
        "${IMGUI_ROOT}/imgui.cpp"
        "${IMGUI_ROOT}/imgui_draw.cpp"
        "${IMGUI_ROOT}/imgui_tables.cpp"
        "${IMGUI_ROOT}/imgui_widgets.cpp"
        "${IMGUI_ROOT}/backends/imgui_impl_win32.cpp"
        "${IMGUI_ROOT}/backends/imgui_impl_dx11.cpp"
    )
else()
    message(STATUS "Dear ImGui not found in dependencies/ directory")
    message(STATUS "GUI will use fallback Win32 implementation. See dependencies/README.md for ImGui installation")
    set(IMGUI_FOUND FALSE)
endif()

# Try to find MinHook in dependencies directory
set(MINHOOK_ROOT "${CMAKE_SOURCE_DIR}/dependencies/minhook")
if(EXISTS "${MINHOOK_ROOT}/include/MinHook.h")
    message(STATUS "Found MinHook at ${MINHOOK_ROOT}")
    set(MINHOOK_FOUND TRUE)
    set(MINHOOK_INCLUDE_DIR "${MINHOOK_ROOT}/include")
    
    # Find the appropriate library file
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64-bit
        find_library(MINHOOK_LIBRARY
            NAMES MinHook.x64 libMinHook.x64
            PATHS "${MINHOOK_ROOT}/lib"
            NO_DEFAULT_PATH
        )
    else()
        # 32-bit
        find_library(MINHOOK_LIBRARY
            NAMES MinHook.x86 libMinHook.x86
            PATHS "${MINHOOK_ROOT}/lib"
            NO_DEFAULT_PATH
        )
    endif()
    
    if(MINHOOK_LIBRARY)
        message(STATUS "Found MinHook library: ${MINHOOK_LIBRARY}")
    else()
        message(STATUS "MinHook library not found, hooking features will be limited")
        set(MINHOOK_FOUND FALSE)
    endif()
else()
    message(STATUS "MinHook not found in dependencies/ directory")
    message(STATUS "Hooking features will be limited. See dependencies/README.md for installation instructions")
    set(MINHOOK_FOUND FALSE)
endif()

# Try to find cpr library via find_package (vcpkg or system install)
find_package(cpr CONFIG QUIET)
if(cpr_FOUND)
    message(STATUS "Found cpr library via find_package")
    set(CPR_FOUND TRUE)
else()
    message(STATUS "cpr library not found. Authentication features will be disabled.")
    message(STATUS "To enable authentication, install cpr via vcpkg: 'vcpkg install cpr'")
    set(CPR_FOUND FALSE)
endif()

# Add subdirectories
add_subdirectory(src/main)
add_subdirectory(src/capture_dll)
