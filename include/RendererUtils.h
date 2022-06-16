#include <d3d11.h>

namespace Utils
{
    bool compileShader(const WCHAR* filepath, const char* entry, const char* target, ID3DBlob** outShader);
}
