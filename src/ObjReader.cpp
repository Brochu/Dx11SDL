#include "ObjReader.h"

#include <fstream>
#include <sstream>

namespace ObjReader
{
    struct TempBuffers
    {
        std::vector<DirectX::XMFLOAT3> positions;
        std::vector<DirectX::XMFLOAT2> uvs;
        std::vector<DirectX::XMFLOAT3> norms;
    };

    struct VertexComponents
    {
        uint16_t posidx;
        uint16_t uvidx;
        uint16_t normidx;

        VertexComponents(uint16_t pIdx, uint16_t uIdx, uint16_t nIdx)
        {
            posidx = (uint16_t)(pIdx - 1);
            uvidx = (uint16_t)(uIdx - 1);
            normidx = (uint16_t)(nIdx - 1);
        }
    };

    typedef std::unordered_map<uint64_t, uint16_t> VertexCache;

    // Helpers methods - Start
    void AddVertex(VertexCache& cache, TempBuffers& bufs, VertexComponents&& vcomp, ModelData* outModel)
    {
        uint64_t id = 0;
        id = id | vcomp.posidx;
        id <<= 16;
        id = id | vcomp.uvidx;
        id <<= 16;
        id = id | vcomp.normidx;

        if (cache.find(id) == cache.end())
        {
            outModel->verts.push_back({
                bufs.positions[vcomp.posidx],
                bufs.uvs[vcomp.uvidx],
                bufs.norms[vcomp.normidx]
                });
            cache[id] = outModel->verts.size() - 1;
        }

        outModel->indices.push_back(cache[id]);
    }
    void ReadVertex(VertexCache& cache, TempBuffers& bufs, std::stringstream&& ss, ModelData* pModelData)
    {
        std::string type;
        ss >> type;

        if (type == "v")
        {
            DirectX::XMFLOAT3 vert;
            ss >> vert.x;
            ss >> vert.y;
            ss >> vert.z;

            // Not 100% sure why this is needed, is the OBJ file wrong or am I wrong...
            vert.x *= -1;
            bufs.positions.push_back(vert);
        }
        else if (type == "vt")
        {
            DirectX::XMFLOAT2 uv;
            ss >> uv.x;
            ss >> uv.y;

            uv.y *= -1;
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
                AddVertex(cache, bufs, VertexComponents(pIdx[0], uIdx[0], nIdx[0]), pModelData);
                AddVertex(cache, bufs, VertexComponents(pIdx[2], uIdx[2], nIdx[2]), pModelData);
                AddVertex(cache, bufs, VertexComponents(pIdx[1], uIdx[1], nIdx[1]), pModelData);
            }
            else if (pIdx.size() == 4)
            {
                // One quad case
                // First Tri
                AddVertex(cache, bufs, VertexComponents(pIdx[0], uIdx[0], nIdx[0]), pModelData);
                AddVertex(cache, bufs, VertexComponents(pIdx[1], uIdx[1], nIdx[1]), pModelData);
                AddVertex(cache, bufs, VertexComponents(pIdx[2], uIdx[2], nIdx[2]), pModelData);

                // Second Tri
                AddVertex(cache, bufs, VertexComponents(pIdx[0], uIdx[0], nIdx[0]), pModelData);
                AddVertex(cache, bufs, VertexComponents(pIdx[2], uIdx[2], nIdx[2]), pModelData);
                AddVertex(cache, bufs, VertexComponents(pIdx[3], uIdx[3], nIdx[3]), pModelData);
            }
        }
    }
    // Helpers methods - End

    bool ReadSingleMeshFromFile(const char* filepath, ModelData** ppModelData)
    {
        *ppModelData = new ModelData();

        std::ifstream file(filepath);
        if (!file.good() || !file.is_open() || file.bad()) return false;

        VertexCache vertCache;
        TempBuffers bufs;
        std::string line;
        std::stringstream ss;
        while (getline(file, line))
        {
            ReadVertex(vertCache, bufs, std::stringstream(line), *ppModelData);
        }
        (*ppModelData)->meshes.push_back({ "Mesh0", 0, (*ppModelData)->indices.size() });

        file.close();
        return true;
    }
    bool ReadMultipleMeshFromFile(const char* filepath, ModelData** ppModelData)
    {
        // Read Model information, list of objects
        (*ppModelData) = new ModelData();
        (*ppModelData)->objFilename = filepath;

        std::ifstream file(filepath);
        if (!file.good() || !file.is_open() || file.bad()) return false;

        VertexCache vertCache;
        TempBuffers bufs;
        std::string line;
        while (getline(file, line))
        {
            if (line[0] == 'o')
            {
                std::string currentObjName = line.substr(2);
                if (currentObjName == "EmptyObject") continue;

                while (getline(file, line))
                {
                    if (line.substr(0, 3) == "# o")
                    {
                        MeshData mesh = {};
                        mesh.name = line.substr(4);
                        mesh.indexOffset = (*ppModelData)->indices.size();

                        while (getline(file, line))
                        {
                            if (line.substr(0, 6) == "usemtl")
                            {
                                mesh.textureIndex = (*ppModelData)->matCache[line.substr(7)];
                            }
                            else
                            {
                                ReadVertex(vertCache, bufs, std::stringstream(line), *ppModelData);
                            }
                            if (line.find(mesh.name) != -1) break;
                        }

                        mesh.name = currentObjName + "::" + mesh.name;
                        mesh.indexCount = (*ppModelData)->indices.size() - mesh.indexOffset;
                        (*ppModelData)->meshes.push_back(mesh);
                    }

                    if (line.find(currentObjName) != -1) break;
                }
            }
            else if (line.substr(0, 6) == "mtllib")
            {
                //TODO Find a better way to find the matlib in the same folder as the OBJ
                // This will not work with other OBJ files or folder structure
                std::string path(filepath);
                (*ppModelData)->matFilename = path.substr(0, path.size() - 3) + "mtl";
                ReadMaterialLibrary((*ppModelData)->matFilename.c_str(), *ppModelData);
            }
        }

        file.close();
        return true;
    }

    void DebugModelData(const ModelData& modelData)
    {
        printf("[MODEL] file = %s [%ld verts][%ld idx]\n",
            modelData.objFilename.c_str(),
            modelData.verts.size(),
            modelData.indices.size());

        if (modelData.matFilename.size() != 0)
        {
            printf("\tLinked materials filename = %s\n", modelData.matFilename.c_str());
        }

        printf("\tcontains %ld meshes\n", modelData.meshes.size());
        for (const MeshData& m : modelData.meshes)
        {
            printf("\t%s [index offset = %i][index count = %ld][texture index = %i]\n",
                m.name.c_str(),
                m.indexOffset,
                m.indexCount,
                m.textureIndex);
        }
    }

    bool ReadMaterialLibrary(const char* filepath, ModelData* pModelData)
    {
        // Creates an array of all the materials needed to render the model
        std::ifstream file(filepath);
        if (!file.good() || !file.is_open() || file.bad()) return false;

        std::string line;

        getline(file, line);
        while (getline(file, line))
        {
            if (line.size() <= 0) continue;

            std::string mat = line.substr(7);
            getline(file, line);
            std::string tex = line.substr(7);

            uint8_t i;
            for (i = 0; i < pModelData->texCount; i++)
            {
                if (pModelData->texFiles[i] == tex) break;
            }

            if (i == pModelData->texCount)
            {
                pModelData->texFiles[pModelData->texCount++] = tex;
            }
            pModelData->matCache[mat] = i;
        }

        file.close();
        return true;
    }

    void DebugModelMaterial(const ModelData& modelData)
    {
        printf("[MODEL] file = %s [%i textures]\n",
            modelData.matFilename.c_str(),
            modelData.texCount);

        for (uint8_t i = 0; i < modelData.texCount; i++)
        {
            printf("%s ; ", modelData.texFiles[i].c_str());
        }
        printf("\n---\n");
        for (const auto& entry : modelData.matCache)
        {
            printf("%s (%i) ; ", entry.first.c_str(), entry.second);
        }
        printf("\n");
    }
};