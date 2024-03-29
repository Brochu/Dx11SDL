#include "Dx11Renderer.h"
#include "RendererUtils.h"
#include "ObjReader.h"
#include "App.h"
#include "SDL_pixels.h"

#include <SDL_surface.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>


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

ObjReader::ModelData *model = nullptr;

int Dx11Renderer::Init(HWND hWindow, UINT width, UINT height, const char *scenePath)
{
    // This makes sure we have a swapchain, a device and a context
    PrepareBaseObjects(hWindow);

    // Make sure we have all we need to generate shadows in main pass
    PrepareShadowPass();

    // Prep all needed resources for main pass
    PrepareBasePass(width, height, scenePath);

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

int Dx11Renderer::PrepareBasePass(UINT width, UINT height, const char *scenePath)
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
    if (!Utils::compileShader(L"../../../shaders/basePass.hlsl", "VS_Main", "vs_5_0", &pVs))
        return 1;
    hr = pDevice->CreateVertexShader(pVs->GetBufferPointer(), pVs->GetBufferSize(), NULL, &pVertShader);
    assert(SUCCEEDED(hr));

    ID3DBlob *pPs = NULL;
    // PIXEL SHADER
    if (!Utils::compileShader(L"../../../shaders/basePass.hlsl", "PS_Main", "ps_5_0", &pPs))
        return 1;
    hr = pDevice->CreatePixelShader(pPs->GetBufferPointer(), pPs->GetBufferSize(), NULL, &pPixShader);
    assert(SUCCEEDED(hr));

    D3D11_INPUT_ELEMENT_DESC inputElems[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        //TODO: Add tag to sample the right texture
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

    // Loading the requested scene model
    if (!ObjReader::ReadMultipleMeshFromFile(scenePath, &model))
    {
        // Could not read the model file
        return 1;
    }

    // VERTEX BUFFER AND INDEX DESCRIPTION AND CREATION
    {
        D3D11_BUFFER_DESC vbufDesc = {};
        vbufDesc.ByteWidth = sizeof(ObjReader::Vertex) * model->verts.size();
        vbufDesc.Usage = D3D11_USAGE_DEFAULT;
        vbufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vData = {0};
        vData.pSysMem = model->verts.data();

        hr = pDevice->CreateBuffer(
            &vbufDesc,
            &vData,
            &pVertBuf);

        assert(SUCCEEDED(hr));

        // ==========================
        D3D11_BUFFER_DESC ibufDesc = {};
        ibufDesc.ByteWidth = sizeof(uint16_t) * model->indices.size();
        ibufDesc.Usage = D3D11_USAGE_DEFAULT;
        ibufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA iData = {0};
        iData.pSysMem = model->indices.data();

        hr = pDevice->CreateBuffer(
            &ibufDesc,
            &iData,
            &pIdxBuf);

        assert(SUCCEEDED(hr));
    }

    // Load all needed textures and upload to GPU memory + Create shader resource views
    {
        for(uint8_t i = 0; i < model->texCount; i++)
        {
            //TODO: Find a better way to store the current path, at App level or Renderer level
            //Split path and file
            std::string tempPath(scenePath);
            tempPath = tempPath.substr(0, tempPath.size()-9); //Very hacky
            tempPath.append(model->texFiles[i]);

            SDL_Surface *surf = Application::LoadImageFromFile(tempPath);

            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = surf->w;
            texDesc.Height = surf->h;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;
            D3D11_SUBRESOURCE_DATA texData = {};
            texData.pSysMem = surf->pixels;
            texData.SysMemPitch = surf->pitch;
            texData.SysMemSlicePitch = 0;

            hr = pDevice->CreateTexture2D(&texDesc, &texData, &pTextures[i]);
            assert(SUCCEEDED(hr));

            D3D11_SHADER_RESOURCE_VIEW_DESC srDesc = {};
            srDesc.Format = texDesc.Format;
            srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srDesc.Texture2D.MostDetailedMip = 0;
            srDesc.Texture2D.MipLevels = 1;
            hr = pDevice->CreateShaderResourceView(pTextures[i], &srDesc, &pTextureViews[i]);
            assert(SUCCEEDED(hr));

            SDL_FreeSurface(surf);
        }
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

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    memset(&srvDesc, 0, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = pDevice->CreateShaderResourceView(shadowBuffer, &srvDesc, &pShadowShaderView);
    assert (SUCCEEDED(hr) );

    D3D11_SAMPLER_DESC samplerDesc;
    memset(&samplerDesc, 0, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = pDevice->CreateSamplerState(&samplerDesc, &pShadowSampler);
    assert (SUCCEEDED(hr) );

    ID3DBlob *pVs = NULL;
    // VERTEX SHADER
    if (!Utils::compileShader(L"../../../shaders/shadowPass.hlsl", "VS_Main_Shadow", "vs_5_0", &pVs))
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
    DirectX::XMFLOAT4 focus { focusPos[0], focusPos[1], focusPos[2], 1.f };
    DirectX::XMFLOAT4 up { upDir[0], upDir[1], upDir[2], 0.f };

    DirectX::XMStoreFloat4x4(&newData.view, DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat4(&eye),
        DirectX::XMLoadFloat4(&focus),
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
    DirectX::XMFLOAT4 lPos { lightDir[0], lightDir[1], lightDir[2], 1.f };
    DirectX::XMFLOAT4 lFocus { 0.f, 0.f, 0.f, 1.f };
    DirectX::XMFLOAT4 lUp { lightUp[0], lightUp[1], lightUp[2], 0.f };

    DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat4(&lPos),
        DirectX::XMLoadFloat4(&lFocus),
        DirectX::XMLoadFloat4(&lUp)
    );
    DirectX::XMMATRIX lightPersp = DirectX::XMMatrixOrthographicLH(
        4096,
        4096,
        -1.f,
        farZ * 10
    );
    DirectX::XMMATRIX final = DirectX::XMMatrixMultiply(lightView, lightPersp); // Combine

    LightData newLightData = {};
    DirectX::XMStoreFloat4x4(&newLightData.objectToLight, final);
    newLightData.lightDir = { lightDir[0], lightDir[1], lightDir[2], 0.f };

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
    UINT vertStride = sizeof(ObjReader::Vertex);
    UINT vertOffset = 0;

    pCtx->IASetVertexBuffers(0, 1, &pVertBuf, &vertStride, &vertOffset);
    pCtx->IASetIndexBuffer(pIdxBuf, DXGI_FORMAT_R16_UINT, 0);

    // Shadow Pass
    pCtx->OMSetRenderTargets(0, nullptr, pShadowTarget);

    pCtx->VSSetShader(pShadowShader, NULL, 0);
    pCtx->VSSetConstantBuffers(0, 1, &pConstBuf);
    pCtx->VSSetConstantBuffers(1, 1, &pLightBuf);

    pCtx->PSSetShader(nullptr, NULL, 0); // Empty pixel shader for shadow pass

    //TODO: Combine in one draw call if possible?
    for (const ObjReader::MeshData& m : model->meshes)
    {
        pCtx->DrawIndexed(m.indexCount, m.indexOffset, 0);
    }
    //--------------------

    // Base Pass
    pCtx->OMSetRenderTargets(1, &pRenderTarget, pDepthTarget);

    pCtx->VSSetShader(pVertShader, NULL, 0);
    pCtx->VSSetConstantBuffers(0, 1, &pConstBuf);

    pCtx->PSSetShader(pPixShader, NULL, 0);
    pCtx->PSSetConstantBuffers(1, 1, &pLightBuf);
    pCtx->PSSetSamplers(0, 1, &pShadowSampler);
    pCtx->PSSetShaderResources(0, 1, &pShadowShaderView);

    //TODO: Sort calls by texture index + combine draws with the same tex id
    for (const ObjReader::MeshData& m : model->meshes)
    {
        pCtx->PSSetShaderResources(1, 1, &pTextureViews[m.textureIndex]);
        pCtx->DrawIndexed(m.indexCount, m.indexOffset, 0);
    }
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
    ImGui::DragFloat3("Eye Position", eyePos, 1.f, -500.f, 500.f);
    ImGui::DragFloat3("Look Position", focusPos, 1.f, -1000.f, 1000.f);
    ImGui::DragFloat3("Up Direction", upDir, 0.01f, -1.f, 1.f);

    ImGui::Text("Transform:");
    ImGui::Separator();
    ImGui::DragFloat3("Translate", translation, 0.1f, -100.f, 100.f);
    ImGui::DragFloat3("Rotate", rotation, 0.025f, -6.2832f, 6.2832f);
    ImGui::DragFloat3("Scale", scale, 0.025f, -10.f, 10.f);

    ImGui::Text("Directional Light:");
    ImGui::Separator();
    ImGui::DragFloat3("Light Direction", lightDir, 0.025f, -6.2832f, 6.2832f);
    ImGui::DragFloat3("Light Up Direction", lightUp, 0.01f, -1.f, 1.f);

    ImGui::End();
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Dx11Renderer::Quit()
{
    printf("[RENDER] Quitting Dx11 Renderer\n");

    ImGui_ImplDX11_Shutdown();
    pVertBuf->Release();
    pIdxBuf->Release();
    pConstBuf->Release();
    pLightBuf->Release();
    pInputLayout->Release();

    pPixShader->Release();
    pVertShader->Release();
    pShadowShader->Release();

    depthState->Release();

    pRenderTarget->Release();
    pDepthTarget->Release();

    pShadowTarget->Release();
    pShadowSampler->Release();
    pShadowShaderView->Release();

    for(uint8_t i = 0; i < model->texCount; i++)
    {
        pTextures[i]->Release();
        pTextureViews[i]->Release();
    }

    pCtx->Release();
    pDevice->Release();
    pSwapchain->Release();

    delete model;
    printf("[RENDER] Done quitting Dx11\n");
}
