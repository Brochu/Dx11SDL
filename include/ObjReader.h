#pragma once

#include <vector>
#include <DXM/DirectXMath.h>

namespace ObjReader
{
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

    bool ReadFromFile(const char* filepath, ModelData** outModelData);
    void DebugModelData(const ModelData& modelData);
};
