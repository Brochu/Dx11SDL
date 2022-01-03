#include <string>

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

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

    SDL_Window* window;
    SDL_Surface* surface;
};
