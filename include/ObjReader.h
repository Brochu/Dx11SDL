#pragma once

#include <vector>
#include <DXM/DirectXMath.h>

struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT2 uvs;
    DirectX::XMFLOAT3 norm;
};

struct ModelData
{
    std::vector<Vertex> verts;
};

class ObjReader
{
public:

    static bool ReadFromFile(const char* filepath, ModelData& outModelData);
    static void DebugModelData(const ModelData& modelData);
};
