#include "ObjReader.h"

#include <fstream>
#include <sstream>
#include <stdint.h>
#include <string>

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

    bool ReadSingleMeshFromFile(const char* filepath, MeshData** outMeshData)
    {
        *outMeshData = new MeshData();

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
                    AddVertex(bufs, pIdx[0]-1, uIdx[0]-1, nIdx[0]-1, *outMeshData);
                    AddVertex(bufs, pIdx[1]-1, uIdx[1]-1, nIdx[1]-1, *outMeshData);
                    AddVertex(bufs, pIdx[2]-1, uIdx[2]-1, nIdx[2]-1, *outMeshData);
                }
                else if (pIdx.size() == 4)
                {
                    // One quad case
                    // First Tri
                    AddVertex(bufs, pIdx[0]-1, uIdx[0]-1, nIdx[0]-1, *outMeshData);
                    AddVertex(bufs, pIdx[1]-1, uIdx[1]-1, nIdx[1]-1, *outMeshData);
                    AddVertex(bufs, pIdx[2]-1, uIdx[2]-1, nIdx[2]-1, *outMeshData);

                    // Second Tri
                    AddVertex(bufs, pIdx[0]-1, uIdx[0]-1, nIdx[0]-1, *outMeshData);
                    AddVertex(bufs, pIdx[2]-1, uIdx[2]-1, nIdx[2]-1, *outMeshData);
                    AddVertex(bufs, pIdx[3]-1, uIdx[3]-1, nIdx[3]-1, *outMeshData);
                }
            }
        }

        file.close();
        return true;
    }

    bool ReadModelObjectFromFile(const char* filepath, ModelObject** outObjectData)
    {
        //TODO: Implement this
        // Read list of meshes to make a complete object
        return false;
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

    void DebugModelData(const ModelObject& objectData)
    {
        printf("------------------------------------------\n");
        printf("[OBJ] %ld meshes :\n", objectData.meshes.size());

        for (const MeshData& m : objectData.meshes)
        {
            DebugMeshData(m);
        }
        printf("------------------------------------------\n");
    }
};
