#include "nvapi_private.h"
#include "util/util_statuscode.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown *pDevice, IUnknown *pResource, NVDX_ObjectHandle *pHandle) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint(IUnknown *pDev, NVDX_ObjectHandle obj, NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32 *pdwHintValue) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D_BeginResourceRendering(IUnknown *pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D_EndResourceRendering(IUnknown *pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetCurrentSLIState(IUnknown *pDevice, NV_GET_CURRENT_SLI_STATE *pSliState) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D1x_GetGraphicsCapabilities(IUnknown *pDevice,
                                                    NvU32 structVersion,
                                                    NV_D3D1x_GRAPHICS_CAPS *pGraphicsCaps) {
        return Error(__func__);
    }
}
