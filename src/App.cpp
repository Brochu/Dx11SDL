#include "App.h"

Application::Application(const std::string&& name, int w, int h)
    : isRunning(true), AppName(name), width(w), height(h)
{ }

int Application::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("[ERROR] Could not initialize SDL2 library\n");
        return 1;
    }

    //TODO: Create window and surface

    printf("[APP] Finished SDL initialization\n");
    return 0;
}

int Application::Tick()
{
    isRunning = false;
    printf("[APP] Ticking Systems\n");
    //TODO: Need to poll the window for events
    //TODO: Keyboard events
    //TODO: Mouse events

    //TODO: Forward events to the correct systems
    return 0;
}

void Application::Close()
{
    //TODO: Cleanup allocated mem
    //TODO: Destroy widow
    printf("[APP] Done Closing\n");
}
