#include <Dx11Renderer.h>
#include <stdio.h>

#include <winnt.h>
#include <minwinbase.h>

#include <directxmath.h>
#include <d3dcompiler.h>

const char* Dx11Renderer::SHADER_PATH = "";

void Dx11Renderer::Init(HWND hWindow, int width, int height)
{
    printf("[RENDER] Start init flow for Dx11\n");

    HRESULT hr;

    DXGI_MODE_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

    bufferDesc.Width = width;
    bufferDesc.Height = height;
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_SWAP_CHAIN_DESC swapchainDesc;
    ZeroMemory(&swapchainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    swapchainDesc.BufferDesc = bufferDesc;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.BufferCount = 1;
    swapchainDesc.OutputWindow = hWindow;
    swapchainDesc.Windowed = TRUE;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // Create the swapchain
    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
            D3D11_SDK_VERSION, &swapchainDesc, &swapchain, &device, NULL, &ctx);

    ID3D11Texture2D* backbuf;
    hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuf);

    hr = device->CreateRenderTargetView(backbuf, NULL, &renderTarget);
    backbuf->Release();

    ctx->OMSetRenderTargets(1, &renderTarget, NULL);
}

void Dx11Renderer::Update(float delta)
{
}

void Dx11Renderer::Render()
{
    //TODO: Clear screen
    ctx->ClearRenderTargetView(renderTarget, bgColor);

    //TODO: Collect all objs to draw on screen
    //TODO: Prepare pipeline
    //TODO: Draw commands

    swapchain->Present(1, 0);
}

void Dx11Renderer::Quit()
{
    swapchain->Release();
    device->Release();
    ctx->Release();

    printf("[RENDER] Done quitting Dx11\n");
}
