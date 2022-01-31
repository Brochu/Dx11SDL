#include "ObjReader.h"

#include <fstream>
#include <sstream>
#include <stdint.h>
#include <string>

bool ObjReader::ReadFromFile(const char* filepath, ModelData& outModelData)
{
    outModelData = {};

    std::ifstream file(filepath);
    if (!file.good() || !file.is_open() || file.bad()) return false;

    std::vector<DirectX::XMFLOAT3> temp_verts;
    std::vector<DirectX::XMFLOAT2> temp_uvs;
    std::vector<DirectX::XMFLOAT3> temp_norms;

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
            temp_verts.push_back(vert);
        }
        else if (type == "vt")
        {
            DirectX::XMFLOAT2 uv;
            ss >> uv.x;
            ss >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (type == "vn")
        {
            DirectX::XMFLOAT3 norm;
            ss >> norm.x;
            ss >> norm.y;
            ss >> norm.z;
            temp_norms.push_back(norm);
        }
        else if (type == "f")
        {
            uint64_t vertexIdx[3], uvIdx[3], normIdx[3];
            // Tri p0
            ss >> vertexIdx[0]; ss.ignore(1);
            ss >> uvIdx[0]; ss.ignore(1);
            ss >> normIdx[0]; ss.ignore(1);

            // Tri p1
            ss >> vertexIdx[1]; ss.ignore(1);
            ss >> uvIdx[1]; ss.ignore(1);
            ss >> normIdx[1]; ss.ignore(1);

            // Tri p2
            ss >> vertexIdx[2]; ss.ignore(1);
            ss >> uvIdx[2]; ss.ignore(1);
            ss >> normIdx[2]; ss.ignore(1);

            outModelData.verts.push_back({
                temp_verts[vertexIdx[0]-1],
                temp_uvs[uvIdx[0]-1],
                temp_norms[normIdx[0]-1]
            });

            outModelData.verts.push_back({
                temp_verts[vertexIdx[1]-1],
                temp_uvs[uvIdx[1]-1],
                temp_norms[normIdx[1]-1]
            });

            outModelData.verts.push_back({
                temp_verts[vertexIdx[2]-1],
                temp_uvs[uvIdx[2]-1],
                temp_norms[normIdx[2]-1]
            });
        }
    }

    file.close();
    return true;
}

void ObjReader::DebugModelData(const ModelData& modelData)
{
    printf("------------------------------------------\n");
    printf("[OBJ] %ld vertices :\n", modelData.verts.size());

    for (const Vertex& v : modelData.verts)
    {
        printf("[VERT] pos = (%f, %f, %f)\n", v.pos.x, v.pos.y, v.pos.z);
    }
    printf("------------------------------------------\n");
}
