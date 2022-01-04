#include "App.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_timer.h"
#include "SDL2/SDL_video.h"

Application::Application(const std::string&& name, int w, int h)
    : isRunning(true), AppName(name), width(w), height(h)
{ }

int Application::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("[ERROR] Could not initialize SDL2 library; error = %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(AppName.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL)
    {
        printf("[ERROR] Could not create SDL2 window; error = %s\n", SDL_GetError());
        return 1;
    }

    surface = SDL_GetWindowSurface(window);
    printf("[APP] Finished SDL initialization\n");
    return 0;
}

int Application::Tick()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        //TODO: Need to poll the window for events
        if (e.type == SDL_QUIT)
            isRunning = false;

        //TODO: Keyboard events
        if (e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
                case SDLK_UP:
                    printf("[APP] Pressed up!\n");
                    break;
                case SDLK_DOWN:
                    printf("[APP] Pressed down!\n");
                    break;
                case SDLK_LEFT:
                    printf("[APP] Pressed left!\n");
                    break;
                case SDLK_RIGHT:
                    printf("[APP] Pressed right!\n");
                    break;
            }
        }

        //TODO: Mouse events
    }

    //TODO: Forward events to the correct systems
    return 0;
}

void Application::Close()
{
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("[APP] Done Closing\n");
}
