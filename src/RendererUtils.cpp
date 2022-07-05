#include "RendererUtils.h"

#include <d3dcompiler.h>

#include <assert.h>
#include <stdio.h>
#include <winerror.h>

bool Utils::compileShader(const WCHAR *filepath, const char *entry, const char *target, ID3DBlob **outShader)
{
    UINT cmpFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    cmpFlags |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* pError = nullptr;

    HRESULT hr = D3DCompileFromFile(
        filepath,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry,
        target,
        cmpFlags,
        0,
        outShader,
        &pError);
    if (FAILED(hr))
    {
        if (pError)
        {
            OutputDebugStringA((char*) pError->GetBufferPointer());
            printf("%s\n", pError->GetBufferPointer());
            pError->Release();
        }
        if ((*outShader) != nullptr) { (*outShader)->Release(); }
        assert(false);
        return false;
    }
    return true;
}
