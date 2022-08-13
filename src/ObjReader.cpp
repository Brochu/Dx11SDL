#include "ObjReader.h"

#include <fstream>
#include <sstream>
#include <stdint.h>

namespace ObjReader
{
    struct TempBuffers
    {
        std::vector<DirectX::XMFLOAT3> positions;
        std::vector<DirectX::XMFLOAT2> uvs;
        std::vector<DirectX::XMFLOAT3> norms;
    };

    // Helpers methods - Start
    void AddVertex(TempBuffers& bufs, uint64_t pIdx, uint64_t uIdx, uint64_t nIdx, MeshData* out)
    {
        out->verts.push_back({ bufs.positions[pIdx], bufs.uvs[uIdx], bufs.norms[nIdx] });
    }
    // Helpers methods - End

    bool ReadSingleMeshFromFile(const char* filepath, MeshData** ppMeshData)
    {
        *ppMeshData = new MeshData();

        std::ifstream file(filepath);
        if (!file.good() || !file.is_open() || file.bad()) return false;

        TempBuffers bufs;
        std::string line;
        std::stringstream ss;
        while (getline(file, line))
        {
            ss = std::stringstream(line);
            std::string type;
            ss >> type;

            if (type == "v")
            {
                DirectX::XMFLOAT3 vert;
                ss >> vert.x;
                ss >> vert.y;
                ss >> vert.z;
                bufs.positions.push_back(vert);
            }
            else if (type == "vt")
            {
                DirectX::XMFLOAT2 uv;
                ss >> uv.x;
                ss >> uv.y;
                bufs.uvs.push_back(uv);
            }
            else if (type == "vn")
            {
                DirectX::XMFLOAT3 norm;
                ss >> norm.x;
                ss >> norm.y;
                ss >> norm.z;
                bufs.norms.push_back(norm);
            }
            else if (type == "f")
            {
                std::vector<uint64_t> pIdx, uIdx, nIdx;
                while (ss.rdbuf()->in_avail() > 0)
                {
                    // Handle one set of ids
                    uint64_t p, u, n;
                    ss >> p; ss.ignore(1);
                    ss >> u; ss.ignore(1);
                    ss >> n; ss.ignore(1);

                    pIdx.push_back(p);
                    uIdx.push_back(u);
                    nIdx.push_back(n);
                }

                //TODO: Look into some winding order issues, we can see through the Pagoda model
                if (pIdx.size() == 3)
                {
                    // One tri case
                    AddVertex(bufs, pIdx[0]-1, uIdx[0]-1, nIdx[0]-1, *ppMeshData);
                    AddVertex(bufs, pIdx[1]-1, uIdx[1]-1, nIdx[1]-1, *ppMeshData);
                    AddVertex(bufs, pIdx[2]-1, uIdx[2]-1, nIdx[2]-1, *ppMeshData);
                }
                else if (pIdx.size() == 4)
                {
                    // One quad case
                    // First Tri
                    AddVertex(bufs, pIdx[0]-1, uIdx[0]-1, nIdx[0]-1, *ppMeshData);
                    AddVertex(bufs, pIdx[1]-1, uIdx[1]-1, nIdx[1]-1, *ppMeshData);
                    AddVertex(bufs, pIdx[2]-1, uIdx[2]-1, nIdx[2]-1, *ppMeshData);

                    // Second Tri
                    AddVertex(bufs, pIdx[0]-1, uIdx[0]-1, nIdx[0]-1, *ppMeshData);
                    AddVertex(bufs, pIdx[2]-1, uIdx[2]-1, nIdx[2]-1, *ppMeshData);
                    AddVertex(bufs, pIdx[3]-1, uIdx[3]-1, nIdx[3]-1, *ppMeshData);
                }
            }
        }

        file.close();
        return true;
    }
    void DebugMeshData(const MeshData& meshData)
    {
        printf("------------------------------------------\n");
        printf("[MESH] %ld vertices :\n", meshData.verts.size());

        for (const Vertex& v : meshData.verts)
        {
            printf("[VERT] pos = (%f, %f, %f)\n", v.pos.x, v.pos.y, v.pos.z);
        }
        printf("------------------------------------------\n");
    }

    bool ReadModelFromFile(const char* filepath, ModelData** ppModelData)
    {
        //TODO: Implement this
        // Read Model information, list of objects

        (*ppModelData) = new ModelData();
        (*ppModelData)->filename = filepath;

        return false;
    }
    bool ReadObjectForModel(std::ifstream& file, ModelData* outModelData)
    {
        //TODO: Implement this
        // Read all the info needed to represent an object from the model
        return false;
    }
    bool ReadMeshForObject(std::ifstream& file, ObjectData* outObjectData)
    {
        //TODO: Implement this
        // Read a single mesh from an obj file containing a full object
        // Read all mesh info between o entries
        // Add the created mesh in the out object param
        return false;
    }

    void DebugModelData(const ModelData& modelData)
    {
        printf("------------------------------------------\n");
        printf("[MODEL] (file = %s) with %ld objects :\n", modelData.filename.c_str() ,modelData.objects.size());

        for (const ObjectData& o : modelData.objects)
        {
            DebugObjectData(o);
        }
        printf("------------------------------------------\n");
    }
    void DebugObjectData(const ObjectData& objectData)
    {
        printf("------------------------------------------\n");
        printf("[OBJ] (name = ) with %ld meshes :\n", objectData.name.c_str() ,objectData.meshes.size());

        for (const MeshData& m : objectData.meshes)
        {
            DebugMeshData(m);
        }
        printf("------------------------------------------\n");
    }
};
