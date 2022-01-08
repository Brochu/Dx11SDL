#include <d3d11.h>
#include <dxgi.h>
#include <directxmath.h>
#include <d3dcompiler.h>

class Dx11Renderer
{

public:
    //Dx11Renderer();

    void Init();
    void Render();
    void Quit();

private:
    IDXGIDevice* device;
    IDXGISwapChain* swapchain;

    static const char* SHADER_PATH;
};
