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
        std::vector<Vertex> verts;
    };

    struct ObjectData
    {
        std::string name;
        std::vector<MeshData> meshes;
    };

    struct ModelData
    {
        std::string filename;
        std::vector<ObjectData> objects;
    };

    bool ReadSingleMeshFromFile(const char* filepath, MeshData** ppMeshData);
    void DebugMeshData(const MeshData& meshData);

    bool ReadModelFromFile(const char* filepath, ModelData** ppModelData);
    bool ReadObjectForModel(std::ifstream& file, ObjectData& outObjectData);
    bool ReadMeshForObject(std::ifstream& file, MeshData& outMeshData);

    void DebugModelData(const ModelData& modelData);
    void DebugObjectData(const ObjectData& objectData);
};
