#pragma once

#include <DirectXMath.h>
#include <string>
#include <vector>
#include <unordered_map>

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

        uint8_t textureIndex = 0;
        uint64_t indexOffset;
        uint64_t indexCount;
    };

    struct ModelData
    {
        std::string objFilename;
        std::string matFilename;

        std::vector<Vertex> verts;
        std::vector<uint16_t> indices;

        std::vector<MeshData> meshes;

        std::unordered_map<std::string, uint8_t> matCache;
        std::string texFiles[256];
        uint8_t texCount = 0;
    };

    bool ReadSingleMeshFromFile(const char* filepath, ModelData** ppModelData);
    bool ReadMultipleMeshFromFile(const char* filepath, ModelData** ppModelData);

    void DebugModelData(const ModelData& modelData);

    bool ReadMaterialLibrary(const char* filepath, ModelData* pModelData);
    void DebugModelMaterial(const ModelData& modelData);
};
