#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

struct ModelData;

class Dx11Renderer
{

public:
    void Init(HWND hWindow, int width, int height);
    void Update(float time, float delta);
    void Render();
    void RenderDebugUI();
    void Quit();

private:
    IDXGISwapChain* swapchain;
    ID3D11Device* device;
    ID3D11DeviceContext* ctx;

    ID3D11RenderTargetView* renderTarget;
    ID3D11Buffer* vertBuf;

    ID3D11VertexShader* vertShader;
    ID3D11PixelShader* pixShader;
    ID3D11InputLayout* inputLayout;

    float bgColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };
    ModelData* cubeModel;
    //TODO: Make sure this model works as well
    //ModelData* houseModel;

    static const char* SHADER_PATH;
    static const char* DATA_PATH;
};
