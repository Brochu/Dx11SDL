#include "Dx11Renderer.h"
#include "ObjReader.h"

#include <d3dcompiler.h>
#include <DXM/DirectXMath.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>

#include <cstdio>
#include <winerror.h>

struct PerFrameData
{
    DirectX::XMFLOAT4 time;
};

static bool compileShader(const WCHAR* filepath, const char* entry, const char* target, ID3DBlob** outShader)
{
    UINT cmpFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    cmpFlags |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* pError = nullptr;

    HRESULT hr = D3DCompileFromFile(
        filepath,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry,
        target,
        cmpFlags,
        0,
        outShader,
        &pError);
    if (FAILED(hr))
    {
        if (pError)
        {
            OutputDebugStringA((char*) pError->GetBufferPointer());
            pError->Release();
        }
        if ((*outShader) != nullptr) { (*outShader)->Release(); }
        assert(false);
        return false;
    }
    return true;
}

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

    ID3DBlob *pVs = NULL;
    // VERTEX SHADER
    if (!compileShader(L"shaders/baseShaders.hlsl", "VS_Main", "vs_5_0", &pVs))
        return 1;
    hr = pDevice->CreateVertexShader(pVs->GetBufferPointer(), pVs->GetBufferSize(), NULL, &pVertShader);
    assert(SUCCEEDED(hr));

    ID3DBlob *pPs = NULL;
    // PIXEL SHADER
    if (!compileShader(L"shaders/baseShaders.hlsl", "PS_Main", "ps_5_0", &pPs))
        return 1;
    hr = pDevice->CreatePixelShader(pPs->GetBufferPointer(), pPs->GetBufferSize(), NULL, &pPixShader);
    assert(SUCCEEDED(hr));

    D3D11_INPUT_ELEMENT_DESC inputElems[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    hr = pDevice->CreateInputLayout(
        inputElems,
        ARRAYSIZE(inputElems),
        pVs->GetBufferPointer(),
        pVs->GetBufferSize(),
        &pInputLayout);
    assert(SUCCEEDED(hr));

    pVs->Release();
    pPs->Release();

    // VERTEX BUFFER DESCRIPTION AND CREATION
    Vertex vertData[] = {
        { { 0.0f,  0.5f,  0.0f }, {0.5f, 1.0f} },
        { { 0.5f, -0.5f,  0.0f }, {1.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.0f }, {0.0f, 0.0f} }
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

    // CONSTANT BUFFER DESCRIPTION AND CREATION
    {
        PerFrameData perFrame = {};

        D3D11_BUFFER_DESC perFrameDesc = {};
        perFrameDesc.ByteWidth = sizeof(perFrame);
        perFrameDesc.Usage = D3D11_USAGE_DYNAMIC;
        perFrameDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        perFrameDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        perFrameDesc.MiscFlags = 0;
        perFrameDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA pfData = {0};
        pfData.pSysMem = &perFrame;
        pfData.SysMemPitch = 0;
        pfData.SysMemSlicePitch = 0;

        hr = pDevice->CreateBuffer(
            &perFrameDesc,
            &pfData,
            &pConstBuf);
        assert(SUCCEEDED(hr));
    }

    //TODO: Create constant buffer
    //Will hold time data (time + delta) and transform matrices (view, model and projection)

    viewport = {};
    viewport.TopLeftX = 0.f;
    viewport.TopLeftY = 0.f;
    viewport.Width = (float) width;
    viewport.Height = (float) height;
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;

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

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    pCtx->Map(pConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    PerFrameData newData = {};
    newData.time.x = time / 2;
    newData.time.y = time / 10;
    newData.time.z = delta;
    newData.time.w = delta * 2;
    memcpy(mapped.pData, &newData, sizeof(PerFrameData));

    pCtx->Unmap(pConstBuf, 0);
}

void Dx11Renderer::Render()
{
    // CLEAR
    pCtx->ClearRenderTargetView(pRenderTarget, bgColor);

    // RASTER STATE
    pCtx->RSSetViewports(1, &viewport);

    // OUTPUT MERGER
    pCtx->OMSetRenderTargets(1, &pRenderTarget, nullptr);

    // INPUT ASSEMLBLER
    //TODO: Maybe move these values at class level?
    UINT vertStride = sizeof(Vertex);
    UINT vertOffset = 0;

    pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCtx->IASetInputLayout(pInputLayout);
    pCtx->IASetVertexBuffers(0, 1, &pVertBuf, &vertStride, &vertOffset);

    // SHADERS
    pCtx->VSSetShader(pVertShader, NULL, 0);
    pCtx->VSSetConstantBuffers(0, 1, &pConstBuf);

    pCtx->PSSetShader(pPixShader, NULL, 0);

    // DRAW
    UINT vertCount = 3;
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
    pVertBuf->Release();
    pInputLayout->Release();

    pPixShader->Release();
    pVertShader->Release();

    pRenderTarget->Release();

    pCtx->Release();
    pDevice->Release();
    pSwapchain->Release();

    printf("[RENDER] Done quitting Dx11\n");
}
