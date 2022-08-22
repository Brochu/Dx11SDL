#pragma once

#include <DXM/DirectXMath.h>
#include <stdint.h>
#include <string>
#include <unordered_map>
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
        std::string name = "--NoName--";
        std::vector<Vertex> verts;
        std::vector<uint16_t> indices;
    };

    struct ModelData
    {
        std::string filename;
        std::vector<MeshData> meshes;
    };

    bool ReadSingleMeshFromFile(const char* filepath, MeshData** ppMeshData);
    void DebugMeshData(const MeshData& meshData);

    bool ReadModelFromFile(const char* filepath, ModelData** ppModelData);
    void DebugModelData(const ModelData& modelData);
    void MergeModelToSingleMesh(const ModelData& modelData, MeshData** ppMeshData);
};
