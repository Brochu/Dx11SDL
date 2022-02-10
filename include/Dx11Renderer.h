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

    // Dx11 objects
    IDXGISwapChain* pSwapchain = nullptr;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pCtx = nullptr;

    ID3D11RenderTargetView* pRenderTarget = nullptr;

    ID3D11VertexShader* pVertShader = NULL;
    ID3D11PixelShader* pPixShader = NULL;

    ID3D11InputLayout* pInputLayout = nullptr;
    ID3D11Buffer* pVertBuf = nullptr;
    ID3D11Buffer* pConstBuf = nullptr;

    D3D11_VIEWPORT viewport;

    // Default Values
    float bgColor[4] = { 0.f / 255.f, 100.f / 255.f, 130.f / 255.f, 1.f };

    float fovAngleY = 45.f;
    float aspectRatio;
    float nearZ = 0.1;
    float farZ = 1000.f;

    float eyePos[3] = { 0.f, 0.f, -5.f };
    float focusPos[3] = { 0.f, 0.f, 0.f }; // Needs to be converted to look direction
    float upDir[3] = { 0.f, 1.f, 0.f };

    float translation[3] = { 0.f, 0.f, 0.f };
    float rotation[3] = { 0.f, 0.f, 0.f };
    float scale[3] = { 1.f, 1.f, 1.f };

    float lightDirection[3] = { 0.f, 0.f, 1.f };

    ModelData* model;

    // Debug Values
    UINT frameTimeIdx;
    float frameTimes[10];
    float frameRates[10];
};
