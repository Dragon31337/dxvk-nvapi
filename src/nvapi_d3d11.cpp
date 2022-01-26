#include "nvapi_private.h"
#include "d3d11/nvapi_d3d11_device.h"
#include "util/util_statuscode.h"
#include "util/util_op_code.h"
#include "util/util_string.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown *pDeviceOrContext, NvU32 code, bool *supported) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_SetDepthBoundsTest(IUnknown *pDeviceOrContext, NvU32 bEnable, float fMinDepth, float fMaxDepth) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_BeginUAVOverlap(IUnknown *pDeviceOrContext) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_EndUAVOverlap(IUnknown *pDeviceOrContext) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_MultiDrawInstancedIndirect(ID3D11DeviceContext *pDevContext11, NvU32 drawCount, ID3D11Buffer *pBuffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext *pDevContext11, NvU32 drawCount, ID3D11Buffer *pBuffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateCubinComputeShader(ID3D11Device* pDevice, const void* pCubin, NvU32 size, NvU32 blockX, NvU32 blockY, NvU32 blockZ, NVDX_ObjectHandle* phShader) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateCubinComputeShaderWithName(ID3D11Device* pDevice, const void* pCubin, NvU32 size, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* pShaderName, NVDX_ObjectHandle* phShader) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_LaunchCubinShader(ID3D11DeviceContext* pDeviceContext, NVDX_ObjectHandle hShader, NvU32 gridX, NvU32 gridY, NvU32 gridZ, const void* pParams, NvU32 paramSize, const NVDX_ObjectHandle* pReadResources, NvU32 numReadResources, const NVDX_ObjectHandle* pWriteResources, NvU32 numWriteResources) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_DestroyCubinComputeShader(ID3D11Device* pDevice, NVDX_ObjectHandle hShader) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_IsFatbinPTXSupported(ID3D11Device* pDevice, bool* pSupported) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateUnorderedAccessView(ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAV, NvU32* pDriverHandle) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateShaderResourceView(ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRV, NvU32* pDriverHandle) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_GetResourceHandle(ID3D11Device* pDevice, ID3D11Resource* pResource, NVDX_ObjectHandle* phObject) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_GetResourceGPUVirtualAddress(ID3D11Device* pDevice, const NVDX_ObjectHandle hResource, NvU64* pGpuVA) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_GetResourceGPUVirtualAddressEx(ID3D11Device* pDevice, NV_GET_GPU_VIRTUAL_ADDRESS* pParams) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateSamplerState(ID3D11Device* pDevice, const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState, NvU32* pDriverHandle) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_GetCudaTextureObject(ID3D11Device* pDevice, NvU32 srvDriverHandle, NvU32 samplerDriverHandle, NvU32* pCudaTextureHandle) {
        return Error(__func__);
    }
}
