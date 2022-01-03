#include <stdio.h>
#include "App.h"

int main(int argc, char* argv[])
{
    printf("[MAIN] Init App\n");
    Application app("Dx11-SDL2", 640, 480);
    app.Init();

    while (app.isRunning)
    {
        app.Tick();
    }

    app.Close();

    printf("[MAIN] Terminating\n");
    return 0;
}
