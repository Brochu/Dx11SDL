#include "Dx11Renderer.h"
#include "ObjReader.h"
#include "RendererUtils.h"

#include <d3dcompiler.h>
#include <DXM/DirectXMath.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>

#include <cstdio>
#include <winerror.h>

struct PerFrameData
{
    DirectX::XMFLOAT4 time;

    // Maybe try to combine the transforms on CPU side
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 model;
};

struct LightData
{
    DirectX::XMFLOAT4 lightDir;
    DirectX::XMFLOAT4X4 objectToLight;
};

int Dx11Renderer::Init(HWND hWindow, UINT width, UINT height)
{
    // This makes sure we have a swapchain, a device and a context
    PrepareBaseObjects(hWindow);

    // Make sure we have all we need to generate shadows in main pass
    PrepareShadowPass();

    // Prep all needed resources for main pass
    PrepareBasePass(width, height);

    // Make sure we have created structs for constant buffers
    PrepareCBuffers();

    // Viewport setup
    viewport = {};
    viewport.TopLeftX = 0.f;
    viewport.TopLeftY = 0.f;
    viewport.Width = (float) width;
    viewport.Height = (float) height;
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;
    aspectRatio = viewport.Width / viewport.Height;

    // Create depth state
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = false;
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    pDevice->CreateDepthStencilState(&dsDesc, &depthState);

    // ImGui Init
    ImGui_ImplDX11_Init(pDevice,pCtx);

    printf("[RENDER] Done init Dx11\n");
    return 0;
}

int Dx11Renderer::PrepareBaseObjects(HWND hWindow)
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

    return 0;
}

int Dx11Renderer::PrepareBasePass(UINT width, UINT height)
{
    // Create main render target view
    ID3D11Texture2D* backBuffer;
    HRESULT hr = pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    assert( SUCCEEDED(hr) );

    hr = pDevice->CreateRenderTargetView(backBuffer, 0, &pRenderTarget);
    assert( SUCCEEDED(hr) );
    backBuffer->Release();

    // Create depth target view
    ID3D11Texture2D* depthBuffer = nullptr;

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;
    hr = pDevice->CreateTexture2D(&depthDesc, nullptr, &depthBuffer);
    assert (SUCCEEDED(hr) );

    D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
    depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthViewDesc.Texture2D.MipSlice = 0;

    hr = pDevice->CreateDepthStencilView(depthBuffer, &depthViewDesc, &pDepthTarget);
    assert (SUCCEEDED(hr) );

    depthBuffer->Release();

    ID3DBlob *pVs = NULL;
    // VERTEX SHADER
    if (!Utils::compileShader(L"shaders/basePass.hlsl", "VS_Main", "vs_5_0", &pVs))
        return 1;
    hr = pDevice->CreateVertexShader(pVs->GetBufferPointer(), pVs->GetBufferSize(), NULL, &pVertShader);
    assert(SUCCEEDED(hr));

    ID3DBlob *pPs = NULL;
    // PIXEL SHADER
    if (!Utils::compileShader(L"shaders/basePass.hlsl", "PS_Main", "ps_5_0", &pPs))
        return 1;
    hr = pDevice->CreatePixelShader(pPs->GetBufferPointer(), pPs->GetBufferSize(), NULL, &pPixShader);
    assert(SUCCEEDED(hr));

    D3D11_INPUT_ELEMENT_DESC inputElems[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
    ObjReader::ReadFromFile("data/Pagoda.obj", *model);
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

    return 0;
}

int Dx11Renderer::PrepareShadowPass()
{
    // Move resources creation here, do not bind in advance
    // DO we need a different depth state?

    ID3D11Texture2D* shadowBuffer = nullptr;

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = shadowWidth;
    depthDesc.Height = shadowHeight;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;

    // Also create the shdadow map target texture
    HRESULT hr = pDevice->CreateTexture2D(&depthDesc, nullptr, &shadowBuffer);
    assert (SUCCEEDED(hr) );

    D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
    depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthViewDesc.Texture2D.MipSlice = 0;

    hr = pDevice->CreateDepthStencilView(shadowBuffer, &depthViewDesc, &pShadowTarget);
    assert (SUCCEEDED(hr) );

    //TODO: Maybe add a shder resource view to view debug shadow information
    ID3DBlob *pVs = NULL;
    // VERTEX SHADER
    if (!Utils::compileShader(L"shaders/shadowPass.hlsl", "VS_Main_Shadow", "vs_5_0", &pVs))
        return 1;
    hr = pDevice->CreateVertexShader(pVs->GetBufferPointer(), pVs->GetBufferSize(), NULL, &pShadowShader);
    assert(SUCCEEDED(hr));

    pVs->Release();
    shadowBuffer->Release();

    return 0;
}

int Dx11Renderer::PrepareCBuffers()
{
    HRESULT hr;
    // Split off the handling and creation of constant buffer to other class?
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

    // LIGHT DATA CONSTANT BUFFER
    {
        LightData light = {};

        D3D11_BUFFER_DESC lightDesc = {};
        lightDesc.ByteWidth = sizeof(light);
        lightDesc.Usage = D3D11_USAGE_DYNAMIC;
        lightDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        lightDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        lightDesc.MiscFlags = 0;
        lightDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA data = {0};
        data.pSysMem = &light;
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;

        hr = pDevice->CreateBuffer(&lightDesc, &data, &pLightBuf);
        assert(SUCCEEDED(hr));
    }

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

    DirectX::XMFLOAT4 lookDir { eyePos[0] - focusPos[0], eyePos[1] - focusPos[1], eyePos[2] - focusPos[2], 0.f };
    DirectX::XMVECTOR look = DirectX::XMLoadFloat4(&lookDir);
    look = DirectX::XMVector4Normalize(look);
    DirectX::XMStoreFloat4(&lookDir, look);

    DirectX::XMFLOAT4 up { upDir[0], upDir[1], upDir[2], 0.f };
    DirectX::XMStoreFloat4x4(&newData.view, DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat4(&eye),
        DirectX::XMLoadFloat4(&lookDir),
        DirectX::XMLoadFloat4(&up)
    ));

    DirectX::XMStoreFloat4x4(&newData.projection, DirectX::XMMatrixPerspectiveFovLH(
        fovAngleY,
        aspectRatio,
        nearZ,
        farZ
    ));

    DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
    transform *= DirectX::XMMatrixTranslation(translation[0], translation[1], translation[2]);
    transform *= DirectX::XMMatrixRotationRollPitchYaw(rotation[0], rotation[1], rotation[2]);
    transform *= DirectX::XMMatrixScaling(scale[0], scale[1], scale[2]);
    DirectX::XMStoreFloat4x4(&newData.model, transform);

    // Update light position transformation info
    DirectX::XMFLOAT4 lightPosVec { lightPosition[0], lightPosition[1], lightPosition[2], 1.f };
    DirectX::XMFLOAT4 lightFocusVec { lightFocus[0], lightFocus[1], lightFocus[2], 0.f };
    DirectX::XMFLOAT4 lightUpVec { lightUp[0], lightUp[1], lightUp[2], 0.f };

    DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat4(&lightPosVec),
        DirectX::XMLoadFloat4(&lightFocusVec),
        DirectX::XMLoadFloat4(&lightUpVec)
    );
    DirectX::XMMATRIX lightPersp = DirectX::XMMatrixOrthographicLH(
        128,
        128,
        nearZ,
        farZ
    );
    DirectX::XMMATRIX final = DirectX::XMMatrixMultiply(lightView, lightPersp); // Combine

    LightData newLightData = {};
    DirectX::XMStoreFloat4x4(&newLightData.objectToLight, lightPersp);

    DirectX::XMFLOAT4 lightLookDir { lightPosition[0] - lightFocus[0], lightPosition[1] - lightFocus[1], lightPosition[2] - lightFocus[2], 0.f };
    DirectX::XMVECTOR lightLook = DirectX::XMLoadFloat4(&lightLookDir);
    DirectX::XMVector4Normalize(lightLook);
    DirectX::XMStoreFloat4(&newLightData.lightDir, lightLook);

    // Update constant buffers
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    pCtx->Map(pConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &newData, sizeof(PerFrameData));
    pCtx->Unmap(pConstBuf, 0);

    D3D11_MAPPED_SUBRESOURCE lightMapped = {};
    pCtx->Map(pLightBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &lightMapped);
    memcpy(lightMapped.pData, &newLightData, sizeof(LightData));
    pCtx->Unmap(pLightBuf, 0);
}

void Dx11Renderer::Render()
{
    // Clear render targets
    pCtx->ClearRenderTargetView(pRenderTarget, bgColor);
    pCtx->ClearDepthStencilView(pDepthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    pCtx->ClearDepthStencilView(pShadowTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // General setup for all passes
    pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCtx->IASetInputLayout(pInputLayout);
    pCtx->RSSetViewports(1, &viewport);
    pCtx->OMSetDepthStencilState(depthState, 0);

    //TODO: Move this to model data
    UINT vertStride = sizeof(Vertex);
    UINT vertOffset = 0;
    UINT vertCount = model->verts.size();

    pCtx->IASetVertexBuffers(0, 1, &pVertBuf, &vertStride, &vertOffset);

    // Shadow Pass
    pCtx->OMSetRenderTargets(1, &pRenderTarget, pDepthTarget);

    pCtx->VSSetShader(pShadowShader, NULL, 0);
    pCtx->VSSetConstantBuffers(0, 1, &pConstBuf);
    pCtx->VSSetConstantBuffers(1, 1, &pLightBuf);

    pCtx->PSSetShader(pPixShader, NULL, 0); // Empty pixel shader for shadow pass //TEMP TEST

    pCtx->Draw(vertCount, 0);
    //TODO: Bring back normals and uvs for shadow pass vertex shader... debug in renderdoc again
    //--------------------

    // Base Pass
    //pCtx->OMSetRenderTargets(1, &pRenderTarget, pDepthTarget);

    //pCtx->VSSetShader(pVertShader, NULL, 0);
    //pCtx->VSSetConstantBuffers(0, 1, &pConstBuf);

    //pCtx->PSSetShader(pPixShader, NULL, 0);
    //pCtx->PSSetConstantBuffers(1, 1, &pLightBuf);

    //pCtx->Draw(vertCount, 0);
    //--------------------

    // UI Pass
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
    ImGui::DragFloat("Fov Angle Y", &fovAngleY, 0.2f, 15.f, 85.f);
    ImGui::DragFloat("Near Z", &nearZ, 0.01f, 0.1f, 1.f);
    ImGui::DragFloat("Far Z", &farZ, 10.f, 1000.f, 10000.f);

    ImGui::Text("View:");
    ImGui::Separator();
    ImGui::DragFloat3("Eye Position", eyePos, 1.f, -100.f, 100.f);
    ImGui::DragFloat3("Look Position", focusPos, 1.f, -100.f, 100.f);
    ImGui::DragFloat3("Up Direction", upDir, 0.01f, -1.f, 1.f); //TODO: Change this to a drop down with 8 axis

    ImGui::Text("Transform:");
    ImGui::Separator();
    ImGui::DragFloat3("Translate", translation, 0.1f, -10.f, 10.f);
    ImGui::DragFloat3("Rotate", rotation, 0.1f, 0.f, 10.f);
    ImGui::DragFloat3("Scale", scale, 0.1f, -100.f, 100.f);

    ImGui::Text("Directional Light:");
    ImGui::Separator();
    //TODO: Add debug values to test different light directions

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
    pShadowShader->Release();

    pRenderTarget->Release();

    pCtx->Release();
    pDevice->Release();
    pSwapchain->Release();

    delete model;
    printf("[RENDER] Done quitting Dx11\n");
}
