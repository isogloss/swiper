cmake_minimum_required(VERSION 3.15)

# Define the executable target
add_executable(swiper
    main.cpp
    injector.cpp
)

# Add include directories
target_include_directories(swiper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link Windows libraries
target_link_libraries(swiper PRIVATE
    kernel32
    user32
    gdi32
)

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
