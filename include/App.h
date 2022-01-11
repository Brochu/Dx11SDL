#pragma once

#include <string>

class SDL_Window;
class SDL_Surface;
class Dx11Renderer;

class Application
{

public:
    Application(const std::string&& name, int w, int h);

    int Init();
    int Tick();
    void Close();

    bool isRunning;
    std::string AppName;
    int width;
    int height;

private:

    SDL_Window* window = nullptr;
    SDL_Surface* surface = nullptr;
    Dx11Renderer* render = nullptr;

    uint64_t startTicks = 0;
    uint64_t prevTicks = 0;
};
