#include "App.h"
#include "SDL2/SDL_timer.h"

Application::Application(const std::string&& name, int w, int h)
    : isRunning(true), AppName(name), width(w), height(h)
{ }

int Application::Init()
{
    printf("[APP] Start Init Flow");
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("[ERROR] Could not initialize SDL2 library; error = %s\n", SDL_GetError());
        return 1;
    }

    // Start tracking time spent in the app
    startTicks = prevTicks = SDL_GetTicks64();

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

    // Getting window raw instance to prepare Dx11
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);

    render = new Dx11Renderer();
    render->Init(wmInfo.info.win.window, width, height);
    printf("[APP] Finished Dx11 initialization\n");
    return 0;
}

int Application::Tick()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
            isRunning = false;

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

        if (e.type == SDL_MOUSEBUTTONDOWN)
        {
            printf("[APP] Mouse button %i pressed!\n", e.button.button);
        }
    }

    //TODO: Split event handling if different function
    //TODO: Collect all state changes with events to send to systems
    //TODO: Forward events to the correct systems

    // Time tracking
    uint64_t currentTicks = SDL_GetTicks64();
    float time = (currentTicks - startTicks) / 1000.f;
    float deltaTime = (currentTicks - prevTicks) / 1000.f;
    prevTicks = currentTicks;
    //printf("[APP][TIMES] Time = %f; DeltaTime = %f\n", time, deltaTime);

    render->Update(deltaTime);
    render->Render();
    return 0;
}

void Application::Close()
{
    SDL_DestroyWindow(window);
    SDL_Quit();

    render->Quit();
    delete render;
    printf("[APP] Done Closing\n");
}
