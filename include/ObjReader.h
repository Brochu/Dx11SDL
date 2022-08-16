#pragma once

#include <DXM/DirectXMath.h>
#include <stdint.h>
#include <string>
#include <vector>

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
        std::string name;
        std::vector<Vertex> verts;
    };

    struct ModelData
    {
        std::string filename;
        std::vector<MeshData> meshes;
    };

    bool ReadSingleMeshFromFile(const char* filepath, MeshData** ppMeshData);
    void DebugMeshData(const MeshData& meshData);

    bool ReadModelFromFile(const char* filepath, ModelData** ppModelData);
    bool ReadMeshForModel(std::ifstream& file, MeshData& outMeshData);

    void DebugModelData(const ModelData& modelData);
};
