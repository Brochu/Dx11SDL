#pragma once

#include <string>

class SDL_Window;
class SDL_Surface;
//class Dx11Renderer;

class Application
{

public:
    Application(const std::string&& name, int w, int h);

    int Init(const char* scenePath);
    int Tick();
    void Close();

    static SDL_Surface* LoadImageFromFile(std::string filepath);

    bool isRunning;
    std::string AppName;
    int width;
    int height;

private:
    std::string objPath;

    uint64_t startTicks = 0;
    uint64_t prevTicks = 0;

    SDL_Window* window = nullptr;
    SDL_Surface* surface = nullptr;
    //Dx11Renderer* render = nullptr;
};
