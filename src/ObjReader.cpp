#include "ObjReader.h"

#include <DirectXMath.h>
#include <fstream>
#include <sstream>
#include <string>

bool ObjReader::ReadFromFile(const char* filepath, ModelData& outModelData)
{
    std::ifstream file(filepath);
    if (!file.good()) return false;

    outModelData = {};

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
            outModelData.vertices.push_back(vert);
        }
        else if (type == "vt")
        {
            DirectX::XMFLOAT2 uv;
            ss >> uv.x;
            ss >> uv.y;
            outModelData.uvs.push_back(uv);
        }
        else if (type == "vn")
        {
            DirectX::XMFLOAT3 norm;
            ss >> norm.x;
            ss >> norm.y;
            ss >> norm.z;
            outModelData.normals.push_back(norm);
        }
    }

    file.close();
    return true;
}

void ObjReader::DebugModelData(const ModelData& modelData)
{
    printf("[OBJ] vert = %ld, uvs = %ld, normals = %ld\n",
        modelData.vertices.size(),
        modelData.uvs.size(),
        modelData.normals.size()
    );
}
