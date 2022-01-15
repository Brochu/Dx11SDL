#pragma once

#include <d3d11.h>

struct ModelData;

class Dx11Renderer
{

public:
    int Init(HWND hWindow, UINT width, UINT height);
    void Update(float time, float delta);
    void Render();
    void RenderDebugUI();
    void Quit();

private:
    float bgColor[4] = { 0x64 / 255.f, 0x95 / 255.f, 0xed / 255.f, 1.f };

};
