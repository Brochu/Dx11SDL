#include <windef.h>

#include <d3d11.h>
#include <dxgi.h>

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

    float bgColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };

    static const char* SHADER_PATH;
};
