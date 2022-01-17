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
    float bgColor[4] = { 0.f / 255.f, 100.f / 255.f, 130.f / 255.f, 1.f };

};
