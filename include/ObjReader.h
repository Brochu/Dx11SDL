#pragma once

#include <DXM/DirectXMath.h>
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
        std::string name = "--NoName--";
        std::string materialName = "";

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

        std::string matNames[1024];
        std::string texFiles[1024];
        uint64_t matCount = 0;
    };

    bool ReadSingleMeshFromFile(const char* filepath, ModelData** ppModelData);
    bool ReadMultipleMeshFromFile(const char* filepath, ModelData** ppModelData);

    void DebugModelData(const ModelData& modelData);

    bool ReadMaterialLibrary(const char* filepath, ModelData* pModelData);
    void DebugModelMaterial(const ModelData& modelData);
};
