#pragma once

#include <vector>
#include <DXM/DirectXMath.h>

struct TempBuffers
{
    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT2> uvs;
    std::vector<DirectX::XMFLOAT3> norms;
};

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
