cmake_minimum_required(VERSION 3.15)

# Define the executable target
add_executable(swiper
    main.cpp
    injector.cpp
)

# Add UI sources based on ImGui availability
if(IMGUI_FOUND)
    target_sources(swiper PRIVATE 
        ${IMGUI_SOURCES}
        imgui_ui.cpp
    )
    target_compile_definitions(swiper PRIVATE IMGUI_AVAILABLE)
    message(STATUS "swiper: ImGui support enabled")
else()
    target_sources(swiper PRIVATE ui.cpp)
    message(STATUS "swiper: Using fallback Win32 UI")
endif()

# Add include directories
target_include_directories(swiper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(IMGUI_FOUND)
    target_include_directories(swiper PRIVATE 
        ${IMGUI_INCLUDE_DIR}
        ${IMGUI_INCLUDE_DIR}/backends
    )
endif()

# Link Windows libraries
target_link_libraries(swiper PRIVATE
    kernel32
    user32
    gdi32
    comctl32
    d3d11
    dxgi
)

# Conditionally link cpr if found
if(CPR_FOUND)
    target_link_libraries(swiper PRIVATE cpr::cpr)
    target_compile_definitions(swiper PRIVATE CPR_AVAILABLE)
    message(STATUS "swiper: Authentication support enabled")
else()
    message(STATUS "swiper: Building without authentication support")
endif()

# Set output name
set_target_properties(swiper PROPERTIES
    OUTPUT_NAME "swiper"
)

# Windows-specific settings
if(MSVC)
    target_compile_options(swiper PRIVATE /W4)
    # Set subsystem to Windows (GUI application)
    set_target_properties(swiper PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()
