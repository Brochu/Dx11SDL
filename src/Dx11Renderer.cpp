#include <Dx11Renderer.h>

#include <stdio.h>

const char* Dx11Renderer::SHADER_PATH = "";

void Dx11Renderer::Init()
{
    printf("[RENDER] Start init flow for Dx11");

    //TODO: Prepare all objects needed for rendering
}

void Dx11Renderer::Render()
{
    //TODO: Clear screen
    //TODO: Collect all objs to draw on screen
    //TODO: Prepare pipeline
    //TODO: Draw commands
}

void Dx11Renderer::Quit()
{
    //TODO: Clear all allocations
    //TODO: Destroy objects created in Init

    printf("[RENDER] Done quitting Dx11");
}
