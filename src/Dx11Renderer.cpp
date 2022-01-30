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

    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 model;
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
    model = new ModelData();
    ObjReader::ReadFromFile("data/Cube.obj", *model);

    std::vector<Vertex> vertData;
    vertData.push_back({ { 0.0f,  0.5f,  0.0f }, { 0.5f, 1.0f } });
    vertData.push_back({ { 0.5f, -0.5f,  0.0f }, { 1.0f, 0.0f } });
    vertData.push_back({ {-0.5f, -0.5f,  0.0f }, { 0.0f, 0.0f } });

    {
        D3D11_BUFFER_DESC vbufDesc = {};
        vbufDesc.ByteWidth = sizeof(Vertex) * model->verts.size();
        vbufDesc.Usage = D3D11_USAGE_DEFAULT;
        vbufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA srData = {0};
        srData.pSysMem = model->verts.data();

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

    viewport = {};
    viewport.TopLeftX = 0.f;
    viewport.TopLeftY = 0.f;
    viewport.Width = (float) width;
    viewport.Height = (float) height;
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;
    aspectRatio = viewport.Width / viewport.Height;

    // ImGui Init
    ImGui_ImplDX11_Init(pDevice,pCtx);

    printf("[RENDER] Done init Dx11\n");

    //TODO: remove this, only debugging the values used for view transform
    DirectX::XMFLOAT4 point (0.f, 0.f, 0.f, 1.f);
    printf("base point: (%f, %f, %f, %f)\n", point.x, point.y, point.z, point.w);
    DirectX::XMVECTOR p = DirectX::XMLoadFloat4(&point);

    DirectX::XMFLOAT4 eye { eyePos[0], eyePos[1], eyePos[2], 0.f };
    DirectX::XMFLOAT4 look { lookPos[0], lookPos[1], lookPos[2], 0.f };
    DirectX::XMFLOAT4 up { upDir[0], upDir[1], upDir[2], 0.f };
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(
        DirectX::XMLoadFloat4(&eye),
        DirectX::XMLoadFloat4(&look),
        DirectX::XMLoadFloat4(&up)
    );
    //printf("--------------------\n");
    //for (int i = 0; i < 4; i++)
    //{
    //    for (int j = 0; j < 4; j++)
    //    {
    //        printf("| %f |", output.m[i][j]);
    //    }
    //    printf("\n");
    //}
    //printf("--------------------\n");

    DirectX::XMVECTOR result = DirectX::XMVector4Transform(p, view);
    DirectX::XMFLOAT4 prime;
    DirectX::XMStoreFloat4(&prime, result);
    printf("transformed point: (%f, %f, %f, %f)\n", prime.x, prime.y, prime.z, prime.w);

    return 0;
}

void Dx11Renderer::Update(float time, float delta)
{
    // Update frame times
    frameTimes[frameTimeIdx] = delta;
    frameRates[frameTimeIdx] = ImGui::GetIO().Framerate;
    frameTimeIdx = (frameTimeIdx + 1) % ARRAYSIZE(frameTimes);

    // Create new struct to hold const buffer new data
    PerFrameData newData = {};
    newData.time.x = time;
    newData.time.y = time / 10;
    newData.time.z = delta;
    newData.time.w = delta * 2;

    // Update transform matrices
    DirectX::XMFLOAT4 eye { eyePos[0], eyePos[1], eyePos[2], 1.f };
    DirectX::XMFLOAT4 look { lookPos[0], lookPos[1], lookPos[2], 0.f };
    DirectX::XMFLOAT4 up { upDir[0], upDir[1], upDir[2], 0.f };
    DirectX::XMStoreFloat4x4(&newData.view, DirectX::XMMatrixLookAtRH(
        DirectX::XMLoadFloat4(&eye),
        DirectX::XMLoadFloat4(&look),
        DirectX::XMLoadFloat4(&up)
    ));

    DirectX::XMStoreFloat4x4(&newData.projection, DirectX::XMMatrixPerspectiveFovRH(
        fovAngleY,
        aspectRatio,
        nearZ,
        farZ
    ));

    //TODO: Deal with model transform
    DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
    transform *= DirectX::XMMatrixTranslation(translation[0], translation[1], translation[2]);
    transform *= DirectX::XMMatrixRotationRollPitchYaw(rotation[0], rotation[1], rotation[2]);
    transform *= DirectX::XMMatrixScaling(scale[0], scale[1], scale[2]);
    DirectX::XMStoreFloat4x4(&newData.model, transform);

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    pCtx->Map(pConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
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
    UINT vertCount = model->verts.size();
    pCtx->Draw(vertCount, 0);

    RenderDebugUI();
    pSwapchain->Present(1, 0);
}

void Dx11Renderer::RenderDebugUI()
{
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Debug");

    ImGui::Text("Info:");
    ImGui::Separator();
    ImGui::PlotLines("Frame Rates", frameRates, ARRAYSIZE(frameRates));
    ImGui::PlotLines("Frame Times", frameTimes, ARRAYSIZE(frameTimes));

    ImGui::Text("Defaults:");
    ImGui::Separator();
    ImGui::ColorEdit4("Clear Color", bgColor);

    ImGui::Text("Projection:");
    ImGui::Separator();
    ImGui::DragFloat("Fov Angle Y", &fovAngleY, 0.5f, 25.f, 180.f);
    ImGui::DragFloat("Near Z", &nearZ, 0.01f, 0.01f, 20.f);
    ImGui::DragFloat("Far Z", &farZ, 1.f, 100.f, 1000.f);

    ImGui::Text("View:");
    ImGui::Separator();
    ImGui::DragFloat3("Eye Position", eyePos, 1.f, -100.f, 100.f);
    ImGui::DragFloat3("Look Position", lookPos, 1.f, -100.f, 100.f);
    ImGui::DragFloat3("Up Direction", upDir, 1.f, -100.f, 100.f);

    ImGui::Text("Transform:");
    ImGui::Separator();
    ImGui::DragFloat3("Translate", translation, 0.1f, -10.f, 10.f);
    ImGui::DragFloat3("Rotate", rotation, 0.1f, 0.f, 10.f);
    ImGui::DragFloat3("Scale", scale, 0.1f, -100.f, 100.f);

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

    delete model;
    printf("[RENDER] Done quitting Dx11\n");
}
