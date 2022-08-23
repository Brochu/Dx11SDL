#include "App.h"
#include "ObjReader.h"

#include <cstring>

int RunModelViewer(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("usage: Dx11SDL2.exe <obj file path>\n");
        return 1;
    }

    printf("[MAIN] Init App\n");
    Application app("Dx11-SDL2", 1280, 720);
    if (app.Init(argv[1]) != 0)
    {
        printf("[MAIN] Could not init application!\n");
    }

    while (app.isRunning)
    {
        if (app.Tick() != 0)
        {
            printf("[MAIN] Error while ticking the application!\n");
            break;
        }
    }

    app.Close();

    printf("[MAIN] Terminating\n");
    return 0;
}

int RunTest(int argc, char* argv[])
{
    // No test at the moment
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc > 1 && strcmp(argv[1], "test") == 0)
    {
        return RunTest(argc, argv);
    }

    return RunModelViewer(argc, argv);
}
