#include "Dx11Renderer.h"
#include "ObjReader.h"

#include <debugapi.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DXM/DirectXMath.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>

#include <cstdio>
#include <winerror.h>

ID3D11Device* pDevice = nullptr;
ID3D11DeviceContext* pCtx = nullptr;
IDXGISwapChain* pSwapchain = nullptr;
ID3D11RenderTargetView* pRenderTarget = nullptr;
ID3D11InputLayout* pInputLayout = nullptr;
ID3D11Buffer* pVertBuf = nullptr;

ID3D11VertexShader* pVertShader = NULL;
ID3D11PixelShader* pPixShader = NULL;

UINT vertStride = 3 * sizeof(float);
UINT vertOffset = 0;
UINT vertCount = 3;

UINT frameTimeIdx;
float frameTimes[10];
float frameRates[10];


int Dx11Renderer::Init(HWND hWindow, UINT width, UINT height)
{
    DXGI_SWAP_CHAIN_DESC scDesc = {0};
    scDesc.BufferDesc.RefreshRate.Numerator = 0;
    scDesc.BufferDesc.RefreshRate.Denominator = 1;
    scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scDesc.SampleDesc.Count = 1;
    scDesc.SampleDesc.Quality = 0;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.BufferCount = 1;
    scDesc.OutputWindow = hWindow;
    scDesc.Windowed = true;

    D3D_FEATURE_LEVEL featLevel;
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        NULL,
        0,
        D3D11_SDK_VERSION,
        &scDesc,
        &pSwapchain,
        &pDevice,
        &featLevel,
        &pCtx);
    assert( hr == S_OK && pSwapchain && pDevice && pCtx );

    ID3D11Texture2D* backBuffer;
    hr = pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    assert( SUCCEEDED(hr) );

    hr = pDevice->CreateRenderTargetView(backBuffer, 0, &pRenderTarget);
    assert( SUCCEEDED(hr) );
    backBuffer->Release();

    UINT cmpFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    cmpFlags |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob *pVs = NULL, *pPs = NULL, *pError = NULL;

    // VERTEX SHADER
    hr = D3DCompileFromFile(
        L"shaders/baseShaders.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VS_Main",
        "vs_5_0",
        cmpFlags,
        0,
        &pVs,
        &pError);
    if (FAILED(hr))
    {
        if (pError)
        {
            OutputDebugStringA((char*) pError->GetBufferPointer());
            pError->Release();
        }
        if (pVs) { pVs->Release(); }
        assert(false);
    }

    // PIXEL SHADEr
    hr = D3DCompileFromFile(
        L"shaders/baseShaders.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PS_Main",
        "ps_5_0",
        cmpFlags,
        0,
        &pPs,
        &pError);
    if (FAILED(hr))
    {
        if (pError)
        {
            OutputDebugStringA((char*) pError->GetBufferPointer());
            pError->Release();
        }
        if (pVs) { pVs->Release(); }
        assert(false);
    }

    hr = pDevice->CreateVertexShader(pVs->GetBufferPointer(), pVs->GetBufferSize(), NULL, &pVertShader);
    assert(SUCCEEDED(hr));
    hr = pDevice->CreatePixelShader(pPs->GetBufferPointer(), pPs->GetBufferSize(), NULL, &pPixShader);
    assert(SUCCEEDED(hr));

    D3D11_INPUT_ELEMENT_DESC inputElems[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = pDevice->CreateInputLayout(
        inputElems,
        ARRAYSIZE(inputElems),
        pVs->GetBufferPointer(),
        pVs->GetBufferSize(),
        &pInputLayout);
    assert(SUCCEEDED(hr));

    // VERTEX BUFFER DESCRIPTION AND CREATION
    float vertData[] = {
         0.0f,  0.5f,  0.0f,
         0.5f, -0.5f,  0.0f,
        -0.5f, -0.5f,  0.0f,
    };

    {
        D3D11_BUFFER_DESC vbufDesc = {};
        vbufDesc.ByteWidth = sizeof(vertData);
        vbufDesc.Usage = D3D11_USAGE_DEFAULT;
        vbufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA srData = {0};
        srData.pSysMem = vertData;

        hr = pDevice->CreateBuffer(
            &vbufDesc,
            &srData,
            &pVertBuf);
        assert(SUCCEEDED(hr));
    }

    // ImGui Init
    ImGui_ImplDX11_Init(pDevice,pCtx);

    printf("[RENDER] Done init Dx11\n");
    return 0;
}

void Dx11Renderer::Update(float time, float delta)
{
    // Update frame times
    frameTimes[frameTimeIdx] = delta;
    frameRates[frameTimeIdx] = ImGui::GetIO().Framerate;
    frameTimeIdx = (frameTimeIdx + 1) % ARRAYSIZE(frameTimes);
}

void Dx11Renderer::Render()
{
    // CLEAR
    pCtx->ClearRenderTargetView(pRenderTarget, bgColor);

    // RASTER STATE
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, 1280.f, 720.f, 0.0f, 1.0f };
    pCtx->RSSetViewports(1, &viewport);

    // OUTPUT MERGER
    pCtx->OMSetRenderTargets(1, &pRenderTarget, nullptr);

    // INPUT ASSEMLBLER
    pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCtx->IASetInputLayout(pInputLayout);
    pCtx->IASetVertexBuffers(0, 1, &pVertBuf, &vertStride, &vertOffset);

    // SHADERS
    pCtx->VSSetShader(pVertShader, NULL, 0);
    pCtx->PSSetShader(pPixShader, NULL, 0);

    // DRAW
    pCtx->Draw(vertCount, 0);

    RenderDebugUI();
    pSwapchain->Present(1, 0);
}

void Dx11Renderer::RenderDebugUI()
{
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug");
    ImGui::PlotLines("Frame Rates", frameRates, ARRAYSIZE(frameRates));
    ImGui::PlotLines("Frame Times", frameTimes, ARRAYSIZE(frameTimes));
    ImGui::Separator();
    ImGui::ColorEdit4("Clear Color", bgColor);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Dx11Renderer::Quit()
{
    ImGui_ImplDX11_Shutdown();
    printf("[RENDER] Done quitting Dx11\n");
}
