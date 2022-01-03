cmake_minimum_required (VERSION 3.10)

project (Dx11SDL2)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 17)

set (src
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
)

set (libs
    SDL2main
    SDL2
)
add_executable (Dx11SDL2 ${src})

target_include_directories (Dx11SDL2 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_directories (Dx11SDL2 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/libs)

target_link_libraries (Dx11SDL2 PRIVATE ${libs})
