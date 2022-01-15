#pragma once

#include <d3d11.h>
#include <dxgi.h>

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
    float bgColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };

};
