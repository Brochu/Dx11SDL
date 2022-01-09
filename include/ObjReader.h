#include <vector>
#include <DirectXMath.h>

typedef struct
{
    std::vector<DirectX::XMFLOAT3> vertices;
    std::vector<DirectX::XMFLOAT2> uvs;
    std::vector<DirectX::XMFLOAT3> normals;
} ModelData;

class ObjReader
{
public:

    static bool ReadFromFile(const char* filepath, ModelData& outModelData);
    static void DebugModelData(const ModelData& modelData);
};
