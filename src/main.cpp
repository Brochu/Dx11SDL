#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

int main(int argc, char* argv[])
{
    printf("[MAIN] SDL Init\n");
    int code = SDL_Init(SDL_INIT_VIDEO);
    printf("[MAIN] return from init = %i\n", code);

    SDL_Window* window = nullptr;
    SDL_Surface* surface = nullptr;

    window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    surface = SDL_GetWindowSurface(window);

    SDL_UpdateWindowSurface(window);
    SDL_Delay(2000);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
