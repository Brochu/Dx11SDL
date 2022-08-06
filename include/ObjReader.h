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

    struct MeshData
    {
        std::vector<Vertex> verts;
    };

    struct ModelObject
    {
        std::vector<MeshData> meshes;
    };

    bool ReadSingleMeshFromFile(const char* filepath, MeshData** outMeshData);
    bool ReadModelObjectFromFile(const char* filepath, ModelObject** outObjectData);

    void DebugMeshData(const MeshData& meshData);
    void DebugModelData(const ModelObject& objectData);
};
