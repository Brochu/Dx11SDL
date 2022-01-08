cmake_minimum_required (VERSION 3.10)

project (Dx11SDL2)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 17)

set (headers
    ${CMAKE_CURRENT_LIST_DIR}/include/App.h
    ${CMAKE_CURRENT_LIST_DIR}/include/Dx11Renderer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/ObjReader.h

    ${CMAKE_CURRENT_LIST_DIR}/imgui/imconfig.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_internal.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imstb_rectpack.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imstb_textedit.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imstb_truetype.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_impl_dx11.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_impl_sdl.h

    ${CMAKE_CURRENT_LIST_DIR}/dxmath/DirectXCollision.h
    ${CMAKE_CURRENT_LIST_DIR}/dxmath/DirectXColors.h
    ${CMAKE_CURRENT_LIST_DIR}/dxmath/DirectXMath.h
    ${CMAKE_CURRENT_LIST_DIR}/dxmath/DirectXPackedVector.h
)
set (source
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/App.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Dx11Renderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ObjReader.cpp

    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_impl_dx11.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_impl_sdl.cpp

    ${CMAKE_CURRENT_LIST_DIR}/dxmath/DirectXCollision.inl
    ${CMAKE_CURRENT_LIST_DIR}/dxmath/DirectXMathConvert.inl
    ${CMAKE_CURRENT_LIST_DIR}/dxmath/DirectXMathMatrix.inl
    ${CMAKE_CURRENT_LIST_DIR}/dxmath/DirectXMathMisc.inl
    ${CMAKE_CURRENT_LIST_DIR}/dxmath/DirectXMathVector.inl
    ${CMAKE_CURRENT_LIST_DIR}/dxmath/DirectXPackedVector.inl
)

set (libs
    SDL2main
    SDL2

    d3d11.lib
    d3dx11.lib
    dxgi.lib
    d3dcompiler.lib
)
add_executable (Dx11SDL2 ${source} ${headers})

target_include_directories (Dx11SDL2
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/imgui
    ${CMAKE_CURRENT_LIST_DIR}/dxmath
)
target_link_directories (Dx11SDL2 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/libs)

target_link_libraries (Dx11SDL2 PRIVATE ${libs})
