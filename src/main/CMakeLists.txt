cmake_minimum_required(VERSION 3.15)

# Main executable target
add_executable(swiper
    main.cpp
    injector.cpp
)

# Link against Windows libraries
target_link_libraries(swiper
    PRIVATE
        user32
        kernel32
)

# Set executable properties
set_target_properties(swiper PROPERTIES
    WIN32_EXECUTABLE FALSE  # Console application
    OUTPUT_NAME "swiper"
)

# Include directories
target_include_directories(swiper
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Copy capture.dll to output directory after build (for convenience)
add_custom_command(TARGET swiper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build complete. capture.dll should be in the same directory as swiper.exe"
    COMMENT "Reminder: Ensure capture.dll is available at runtime"
)
