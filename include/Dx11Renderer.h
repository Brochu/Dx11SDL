#pragma once

#include <d3d11.h>
#include <dxgi.h>

struct ModelData;

class Dx11Renderer
{

public:
    int Init(HWND hWindow, int width, int height);
    void Update(float time, float delta);
    void Render();
    void RenderDebugUI();
    void Quit();

private:
    IDXGISwapChain* swapchain;
    ID3D11Device* device;
    ID3D11DeviceContext* ctx;

    ID3D11RenderTargetView* renderTarget;

    ID3D11VertexShader* vertShader;
    ID3D11InputLayout* vertLayout;
    ID3D11PixelShader* pixShader;

    float bgColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };
};
