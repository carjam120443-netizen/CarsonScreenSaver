cmake_minimum_required(VERSION 3.20)
project(CarsonScreenSaver C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(CarsonScreenSaver WIN32
    src/main.c
)

target_compile_definitions(CarsonScreenSaver PRIVATE
    UNICODE
    _UNICODE
)

if(MSVC)
    target_compile_options(CarsonScreenSaver PRIVATE /W4)
endif()

add_custom_command(TARGET CarsonScreenSaver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:CarsonScreenSaver>"
        "$<TARGET_FILE_DIR:CarsonScreenSaver>/CarsonScreenSaver.scr"
)
