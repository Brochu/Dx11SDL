#include <stdio.h>
#include "App.h"

int main(int argc, char* argv[])
{
    printf("[MAIN] Init App\n");
    Application app("Dx11-SDL2", 1280, 720);
    app.Init();

    //TODO: Add some time tracking with frametime? or should this be in the app?
    while (app.isRunning)
    {
        app.Tick();
    }

    app.Close();

    printf("[MAIN] Terminating\n");
    return 0;
}
