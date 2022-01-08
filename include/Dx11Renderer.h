#include <windef.h>
#include <winnt.h>
#include <minwinbase.h>

#include <d3d11.h>
#include <dxgi.h>
#include <directxmath.h>
#include <d3dcompiler.h>

class Dx11Renderer
{

public:
    //Dx11Renderer();

    void Init(HWND hWindow, int width, int height);
    void Render();
    void Quit();

private:
    IDXGISwapChain* swapchain;
    ID3D11Device* device;
    ID3D11DeviceContext* ctx;
    ID3D11RenderTargetView* renderTarget;

    static const char* SHADER_PATH;
};
