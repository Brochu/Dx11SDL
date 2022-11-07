#include "ObjReader.h"
#include <fstream>
#include <string>

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
    void ExtractSubstring(std::string& line, std::string& substring)
    {
        auto pos = line.find(' ');

        if (pos != std::string::npos)
        {
            substring = line.substr(0, pos);
            line = line.substr(pos+1);
        }
        else
        {
            substring = line;
            line = "";
        }
    }
    void ExtractVertex(std::string& line, uint16_t& pid, uint16_t& uid, uint16_t& nid)
    {
        std::string substring;
        ExtractSubstring(line, substring);

        auto pos = line.find('/');
        pid = (uint16_t)std::stoi(line.substr(0, pos));
        line = line.substr(pos+1);

        pos = line.find('/');
        uid = (uint16_t)std::stoi(line.substr(0, pos));
        line = line.substr(pos+1);

        uid = (uint16_t)std::stoi(line);
    }
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
    void ReadVertex(VertexCache& cache, TempBuffers& bufs, std::string& line, ModelData* pModelData)
    {
        std::string type;
        ExtractSubstring(line, type);

        if (type == "v")
        {
            std::string sx, sy, sz;
            ExtractSubstring(line, sx);
            ExtractSubstring(line, sy);
            ExtractSubstring(line, sz);

            DirectX::XMFLOAT3 vert;
            vert.x = std::stof(sx);
            vert.y = std::stof(sy);
            vert.z = std::stof(sz);

            // Not 100% sure why this is needed, is the OBJ file wrong or am I wrong...
            vert.x *= -1;
            bufs.positions.push_back(vert);
        }
        else if (type == "vt")
        {
            std::string sx, sy;
            ExtractSubstring(line, sx);
            ExtractSubstring(line, sy);

            DirectX::XMFLOAT2 uv;
            uv.x = std::stof(sx);
            uv.y = std::stof(sy);

            uv.y *= -1;
            bufs.uvs.push_back(uv);
        }
        else if (type == "vn")
        {
            std::string sx, sy, sz;
            ExtractSubstring(line, sx);
            ExtractSubstring(line, sy);
            ExtractSubstring(line, sz);

            DirectX::XMFLOAT3 norm;
            norm.x = std::stof(sx);
            norm.y = std::stof(sy);
            norm.z = std::stof(sz);

            bufs.norms.push_back(norm);
        }
        else if (type == "f")
        {
            std::vector<uint16_t> pIdx, uIdx, nIdx;
            while (line.length() > 0)
            {
                // Handle one set of ids
                uint16_t p, u, n;
                ExtractVertex(line, p, u, n);

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
        while (getline(file, line))
        {
            ReadVertex(vertCache, bufs, line, *ppModelData);
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

                while(getline(file, line))
                {
                    if (line.substr(0, 3) == "# o")
                    {
                        MeshData mesh = {};
                        mesh.name = line.substr(4);
                        mesh.indexOffset = (*ppModelData)->indices.size();

                        while(getline(file, line))
                        {
                            if (line.substr(0, 6) == "usemtl")
                            {
                                mesh.textureIndex = (*ppModelData)->matCache[line.substr(7)];
                            }
                            else
                            {
                                ReadVertex(vertCache, bufs, line, *ppModelData);
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
                (*ppModelData)->matFilename = path.substr(0, path.size()-3) + "mtl";
                ReadMaterialLibrary((*ppModelData)->matFilename.c_str(), *ppModelData);
            }
        }

        file.close();
        return true;
    }

    void DebugModelData(const ModelData& modelData)
    {
        printf("[MODEL] file = %s [%llu verts][%llu idx]\n",
            modelData.objFilename.c_str(),
            modelData.verts.size(),
            modelData.indices.size());

        if (modelData.matFilename.size() != 0)
        {
            printf("\tLinked materials filename = %s\n", modelData.matFilename.c_str());
        }

        printf("\tcontains %llu meshes\n", modelData.meshes.size());
        for (const MeshData& m : modelData.meshes)
        {
            printf("\t%s [index offset = %llu][index count = %llu][texture index = %i]\n",
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
        while(getline(file, line))
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
        for (const auto& entry :  modelData.matCache)
        {
            printf("%s (%i) ; ", entry.first.c_str(), entry.second);
        }
        printf("\n");
    }
};
