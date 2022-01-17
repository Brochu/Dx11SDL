#pragma once

#include <d3d11.h>
#include <dxgi.h>

class Dx11Renderer
{

public:
    int Init(HWND hWindow, UINT width, UINT height);
    void Update(float time, float delta);
    void Render();
    void RenderDebugUI();
    void Quit();

private:

    IDXGISwapChain* pSwapchain = nullptr;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pCtx = nullptr;

    ID3D11RenderTargetView* pRenderTarget = nullptr;

    ID3D11VertexShader* pVertShader = NULL;
    ID3D11PixelShader* pPixShader = NULL;

    ID3D11InputLayout* pInputLayout = nullptr;
    ID3D11Buffer* pVertBuf = nullptr;

    D3D11_VIEWPORT viewport;

    float bgColor[4] = { 0.f / 255.f, 100.f / 255.f, 130.f / 255.f, 1.f };

    UINT frameTimeIdx;
    float frameTimes[10];
    float frameRates[10];
};
