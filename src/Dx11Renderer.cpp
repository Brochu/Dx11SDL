#include "Dx11Renderer.h"
#include "ObjReader.h"

#include <debugapi.h>
#include <string>
#include <imgui_impl_dx11.h>

#include <DXM/DirectXMath.h>
#include <winerror.h>

static const char* SHADER_PATH = "shaders/";
static const char* DATA_PATH = "data/";

int Dx11Renderer::Init(HWND hWindow, int width, int height)
{
    printf("[RENDER] Start init flow for Dx11\n");

    HRESULT hr;

    // Create description for back buffer
    DXGI_MODE_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));
    bufferDesc.Width = width;
    bufferDesc.Height = height;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;

    // Create description for swap chain
    DXGI_SWAP_CHAIN_DESC swapchainDesc;
    ZeroMemory(&swapchainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    swapchainDesc.BufferCount = 1;
    swapchainDesc.BufferDesc = bufferDesc;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.OutputWindow = hWindow;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.Windowed = TRUE;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // Prepare metadata to create swapchain, device and context
    D3D_FEATURE_LEVEL featLevel;
    D3D_FEATURE_LEVEL featLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    UINT numFeatLevels = ARRAYSIZE(featLevels);

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWARE
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    UINT flags = 0;
    flags |= D3D11_CREATE_DEVICE_DEBUG;

    // Create
    hr = D3D11CreateDeviceAndSwapChain(
            NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            flags,
            featLevels,
            numFeatLevels,
            D3D11_SDK_VERSION,
            &swapchainDesc,
            &swapchain,
            &device,
            &featLevel,
            &ctx);
    if (FAILED(hr))
    {
        printf("[RENDER] Could not create device and swap chain\n");
        return 1;
    }

    // Get and set the render target
    ID3D11Texture2D* backbuf;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuf);

    hr = device->CreateRenderTargetView(backbuf, NULL, &renderTarget);
    if (FAILED(hr))
    {
        printf("[RENDER] Could not create render target view\n");
        return 1;
    }
    backbuf->Release();
    ctx->OMSetRenderTargets(1, &renderTarget, nullptr);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.Width = (FLOAT)width;
    viewport.Height = (FLOAT)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 10.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    ctx->RSSetViewports(1, &viewport);

    // Finish IMGUI setup
    ImGui_ImplDX11_Init(device, ctx);

    return 0;
}

void Dx11Renderer::Update(float time, float delta)
{
}

void Dx11Renderer::Render()
{
    ctx->ClearRenderTargetView(renderTarget, bgColor);

    //TODO: Collect all objs to draw on screen
    //TODO: Prepare pipeline
    //TODO: Draw commands

    RenderDebugUI();
    swapchain->Present(0, 0);
}

void Dx11Renderer::RenderDebugUI()
{
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug");
    ImGui::ColorEdit4("ClearColor", bgColor);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Dx11Renderer::Quit()
{
    ctx->Release();
    device->Release();
    swapchain->Release();

    ImGui_ImplDX11_Shutdown();

    printf("[RENDER] Done quitting Dx11\n");
}
