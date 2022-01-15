#include "Dx11Renderer.h"
#include "ObjReader.h"

#include <debugapi.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DXM/DirectXMath.h>

#include <cstdio>
#include <winerror.h>

ID3D11Device* pDevice = nullptr;
ID3D11DeviceContext* pCtx = nullptr;
IDXGISwapChain* pSwapchain = nullptr;
ID3D11RenderTargetView* pRenderTarget;

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

    printf("[RENDER] Done init Dx11\n");
    return 0;
}

void Dx11Renderer::Update(float time, float delta)
{
}

void Dx11Renderer::Render()
{
}

void Dx11Renderer::Quit()
{
    printf("[RENDER] Done quitting Dx11\n");
}
