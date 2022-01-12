#include "Dx11Renderer.h"
#include "ObjReader.h"

#include <debugapi.h>
#include <string>
#include <imgui_impl_dx11.h>

#include <DXM/DirectXMath.h>
#include <winerror.h>

const char* Dx11Renderer::SHADER_PATH = "shaders/";
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

    // Compile and create shaders
    ID3DBlob* vs_blob = nullptr;
    ID3DBlob* ps_blob = nullptr;
    ID3DBlob* error_blob = nullptr;

    hr = D3DCompileFromFile(L"shaders/baseShaders.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs_main",
        "vs_5_0",
        0,
        0,
        &vs_blob,
        &error_blob);

    if (FAILED(hr))
    {
        if (error_blob)
        {
            OutputDebugStringA( (char*)error_blob->GetBufferPointer() );
            error_blob->Release();
        }
        if (vs_blob) vs_blob->Release();
        assert(false);
    }

    hr = D3DCompileFromFile(L"shaders/baseShaders.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps_main",
        "ps_5_0",
        0,
        0,
        &ps_blob,
        &error_blob);

    if (FAILED(hr))
    {
        if (error_blob)
        {
            OutputDebugStringA( (char*)error_blob->GetBufferPointer() );
            error_blob->Release();
        }
        if (ps_blob) ps_blob->Release();
        assert(false);
    }

    hr = device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &vertShader);
    assert(SUCCEEDED(hr));

    hr = device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &pixShader);
    assert(SUCCEEDED(hr));

    // Define input layouts
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD0", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    device->CreateInputLayout(ied,
        ARRAYSIZE(ied),
        vs_blob->GetBufferPointer(),
        vs_blob->GetBufferSize(),
        &inputLayout);
}

void Dx11Renderer::Update(float time, float delta)
{
}

void Dx11Renderer::Render()
{
    ctx->OMSetRenderTargets(1, &renderTarget, NULL);
    ctx->ClearRenderTargetView(renderTarget, bgColor);

    ctx->VSSetShader(vertShader, nullptr, 0);
    ctx->PSSetShader(pixShader, nullptr, 0);
    ctx->IASetInputLayout(inputLayout);

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    //TODO: Make the width and height available, saved in renderer?
    viewport.Width = 1280;
    viewport.Height = 720;
    ctx->RSSetViewports(1, &viewport);

    //TODO: Collect all objs to draw on screen
    //TODO: Prepare pipeline
    //TODO: Draw commands
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &vertBuf, &stride, &offset);

    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ctx->Draw(cubeModel->verts.size(), 0);

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
