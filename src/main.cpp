#include <stdio.h>
#include <string>

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

const char* AppName = "Dx11-SDL2";

int main(int argc, char* argv[])
{
    printf("[MAIN] Init\n");

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("[ERROR] Could not initialize SDL2 library\n");
        return 1;
    }

    printf("[MAIN] Finished SDL initialization\n");
    return 0;
}
