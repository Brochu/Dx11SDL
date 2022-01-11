#pragma once

#include <vector>
#include <DirectXMath.h>

struct ModelData
{
    std::vector<DirectX::XMFLOAT3> vertices;
    std::vector<DirectX::XMFLOAT2> uvs;
    std::vector<DirectX::XMFLOAT3> normals;
};
