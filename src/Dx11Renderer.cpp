#include "Dx11Renderer.h"
#include "ObjReader.h"

#include <string>
#include <imgui_impl_dx11.h>

#include <DXM/DirectXMath.h>

const char* Dx11Renderer::SHADER_PATH = "";
const char* Dx11Renderer::DATA_PATH = "data/";

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

    // Finish IMGUI setup
    ImGui_ImplDX11_Init(device, ctx);

    // Test loading model data
    cubeModel = new ModelData();
    std::string modelPath = DATA_PATH;
    modelPath += "cube.obj";
    ObjReader::ReadFromFile(modelPath.c_str(), *cubeModel);

    ObjReader::DebugModelData(*cubeModel);

    // Create vertex buffer
    D3D11_BUFFER_DESC bd = {0};
    bd.ByteWidth = sizeof(Vertex) * cubeModel->verts.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA srd = { cubeModel->verts.data(), 0, 0 };

    device->CreateBuffer(&bd, &srd, &vertBuf);
}

void Dx11Renderer::Update(float time, float delta)
{
}

void Dx11Renderer::Render()
{
    ctx->OMSetRenderTargets(1, &renderTarget, NULL);
    ctx->ClearRenderTargetView(renderTarget, bgColor);

    //TODO: Collect all objs to draw on screen
    //TODO: Prepare pipeline
    //TODO: Draw commands

    RenderDebugUI();
    swapchain->Present(1, 0);
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
    vertBuf->Release();
    delete cubeModel;

    ctx->Release();
    device->Release();
    swapchain->Release();

    ImGui_ImplDX11_Shutdown();

    printf("[RENDER] Done quitting Dx11\n");
}
