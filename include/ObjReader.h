#pragma once

struct ModelData;

class ObjReader
{
public:

    static bool ReadFromFile(const char* filepath, ModelData& outModelData);
    static void DebugModelData(const ModelData& modelData);
};
