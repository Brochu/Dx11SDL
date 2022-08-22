#include "App.h"
#include "ObjReader.h"

int main(int argc, char* argv[])
{
    //printf("[MAIN] Init App\n");
    //Application app("Dx11-SDL2", 1280, 720);
    //if (app.Init() != 0)
    //{
    //    printf("[MAIN] Could not init application!\n");
    //}

    //while (app.isRunning)
    //{
    //    if (app.Tick() != 0)
    //    {
    //        printf("[MAIN] Error while ticking the application!\n");
    //        break;
    //    }
    //}

    //app.Close();

    //printf("[MAIN] Terminating\n");
    //return 0;

    // TESTING MODEL IMPORT
    ObjReader::ModelData* model;
    ObjReader::ReadModelFromFile("data/Volcan/model.obj", &model);
    ObjReader::DebugModelData(*model);

    //TODO: Try to combine the submeshes into one mesh data object for easier rendering

    delete model;
    return 0;
}
