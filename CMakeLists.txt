cmake_minimum_required(VERSION 3.10)
project(RenderObj CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SDL2 REQUIRED)

add_executable(render_obj
    src/main.cpp
    src/framebuffer.cpp
    src/triangle.cpp
    src/obj_loader.cpp
)

target_include_directories(render_obj PRIVATE ${SDL2_INCLUDE_DIRS})

if(TARGET SDL2::SDL2main)
    target_link_libraries(render_obj PRIVATE SDL2::SDL2main)
endif()

if(TARGET SDL2::SDL2)
    target_link_libraries(render_obj PRIVATE SDL2::SDL2)
else()
    target_link_libraries(render_obj PRIVATE ${SDL2_LIBRARIES})
endif()

# Copia el modelo junto al ejecutable para que la ruta relativa
# "NaveEspacial.obj" funcione al ejecutar desde la carpeta build.
add_custom_command(TARGET render_obj POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/NaveEspacial.obj" "$<TARGET_FILE_DIR:render_obj>/NaveEspacial.obj"
)
