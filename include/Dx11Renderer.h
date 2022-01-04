#include <d3d11.h>
#include <dxgi.h>

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

};
