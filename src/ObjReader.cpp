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

    typedef std::unordered_map<uint64_t, uint16_t> VertexCache;

    // Helpers methods - Start
    void AddVertex(VertexCache& cache, TempBuffers& bufs, uint16_t pIdx, uint16_t uIdx, uint16_t nIdx, MeshData* out)
    {
        uint64_t id = 0;
        id = id | pIdx;
        id <<= 16;
        id = id | uIdx;
        id <<= 16;
        id = id | nIdx;

        if (cache.find(id) == cache.end())
        {
            out->verts.push_back({ bufs.positions[pIdx], bufs.uvs[uIdx], bufs.norms[nIdx] });
            cache[id] = out->verts.size() - 1;
        }

        out->indices.push_back(cache[id]);
    }
    void ReadVertex(VertexCache& cache, TempBuffers& bufs, std::stringstream&& ss, MeshData* ppMeshData)
    {
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
            std::vector<uint16_t> pIdx, uIdx, nIdx;
            while (ss.rdbuf()->in_avail() > 0)
            {
                // Handle one set of ids
                uint16_t p, u, n;
                ss >> p; ss.ignore(1);
                ss >> u; ss.ignore(1);
                ss >> n; ss.ignore(1);

                pIdx.push_back(p);
                uIdx.push_back(u);
                nIdx.push_back(n);
            }

            if (pIdx.size() == 3)
            {
                // One tri case
                AddVertex(cache, bufs, pIdx[0]-1, uIdx[0]-1, nIdx[0]-1, ppMeshData);
                AddVertex(cache, bufs, pIdx[1]-1, uIdx[1]-1, nIdx[1]-1, ppMeshData);
                AddVertex(cache, bufs, pIdx[2]-1, uIdx[2]-1, nIdx[2]-1, ppMeshData);
            }
            else if (pIdx.size() == 4)
            {
                // One quad case
                // First Tri
                AddVertex(cache, bufs, pIdx[0]-1, uIdx[0]-1, nIdx[0]-1, ppMeshData);
                AddVertex(cache, bufs, pIdx[1]-1, uIdx[1]-1, nIdx[1]-1, ppMeshData);
                AddVertex(cache, bufs, pIdx[2]-1, uIdx[2]-1, nIdx[2]-1, ppMeshData);

                // Second Tri
                AddVertex(cache, bufs, pIdx[0]-1, uIdx[0]-1, nIdx[0]-1, ppMeshData);
                AddVertex(cache, bufs, pIdx[2]-1, uIdx[2]-1, nIdx[2]-1, ppMeshData);
                AddVertex(cache, bufs, pIdx[3]-1, uIdx[3]-1, nIdx[3]-1, ppMeshData);
            }
        }
    }
    // Helpers methods - End

    bool ReadSingleMeshFromFile(const char* filepath, MeshData** ppMeshData)
    {
        *ppMeshData = new MeshData();

        std::ifstream file(filepath);
        if (!file.good() || !file.is_open() || file.bad()) return false;

        VertexCache vertCache;
        TempBuffers bufs;
        std::string line;
        std::stringstream ss;
        while (getline(file, line))
        {
            ReadVertex(vertCache, bufs, std::stringstream(line), *ppMeshData);
        }

        file.close();
        return true;
    }
    void DebugMeshData(const MeshData& meshData)
    {
        printf("[MESH] %s with %ld vertices & %ld indices:\n",
            meshData.name.c_str(),
            meshData.verts.size(),
            meshData.indices.size());

        //for (const uint16_t& i : meshData.indices)
        //{
        //    printf("[VERTEX] index at %i\n", i);
        //}
    }

    bool ReadModelFromFile(const char* filepath, ModelData** ppModelData)
    {
        // Read Model information, list of objects
        (*ppModelData) = new ModelData();
        (*ppModelData)->filename = filepath;

        std::ifstream file(filepath);
        if (!file.good() || !file.is_open() || file.bad()) return false;

        VertexCache vertCache;
        TempBuffers bufs;
        std::string line;
        while (getline(file, line))
        {
            //TODO: Parsing the material file if found
            if (line[0] == 'o')
            {
                MeshData mesh;
                mesh.name = line.substr(2);
                if (mesh.name == "EmptyObject") continue;

                while(getline(file, line))
                {
                    ReadVertex(vertCache, bufs, std::stringstream(line), &mesh);

                    if (line.find(mesh.name) != -1) break;
                }
                (*ppModelData)->meshes.push_back(mesh);
            }
        }

        return true;
    }

    void DebugModelData(const ModelData& modelData)
    {
        printf("[MODEL] (file = %s) with %ld meshes :\n", modelData.filename.c_str() ,modelData.meshes.size());

        for (const MeshData& m : modelData.meshes)
        {
            DebugMeshData(m);
        }
    }

    void MergeModelToSingleMesh(const ModelData& modelData, MeshData** ppMeshData)
    {
        // Combine all sub meshes found in the model into one for easier rendering
        (*ppMeshData) = new MeshData();
        (*ppMeshData)->name = modelData.filename;

        for (ObjReader::MeshData m : modelData.meshes)
        {
            for (uint16_t i : m.indices)
            {
                (*ppMeshData)->indices.push_back((uint16_t)i + (*ppMeshData)->verts.size());
            }
            for (ObjReader::Vertex v : m.verts)
            {
                (*ppMeshData)->verts.push_back(v);
            }
        }
    }
};

