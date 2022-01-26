#include "nvapi_private.h"
#include "nvapi_static.h"
#include "util/util_statuscode.h"
#include "util/util_error.h"
#include "util/util_string.h"
#include "util/util_env.h"
#include "util/util_log.h"
#include "../version.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // __GNUC__

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle nvGPUHandle[NVAPI_MAX_LOGICAL_GPUS], NvU32 *pGpuCount) {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_EnumTCCPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount)  {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_GetGPUIDfromPhysicalGPU(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pGpuId) {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUFromGPUID(NvU32 gpuId, NvPhysicalGpuHandle *hPhysicalGpu) {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_GetDisplayDriverVersion(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_VERSION *pVersion) {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromLogicalGPU(NvLogicalGpuHandle hLogicalGPU,NvPhysicalGpuHandle hPhysicalGPU[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromDisplay(NvDisplayHandle hNvDisp, NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_EnumNvidiaDisplayHandle(NvU32 thisEnum, NvDisplayHandle *pNvDispHandle) {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_EnumNvidiaUnAttachedDisplayHandle(NvU32 thisEnum, NvUnAttachedDisplayHandle *pNvUnAttachedDispHandle) {
        // DXVK does not know about unattached displays
        return EndEnumeration(str::format(__func__, " ", thisEnum));
    }

    NvAPI_Status __cdecl NvAPI_GetAssociatedNvidiaDisplayName(NvDisplayHandle NvDispHandle, NvAPI_ShortString szDisplayName) {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_GetInterfaceVersionString(NvAPI_ShortString szDesc) {
        constexpr auto n = __func__;

        return Error(n);
    }

    NvAPI_Status __cdecl NvAPI_GetErrorMessage(NvAPI_Status nr, NvAPI_ShortString szDesc) {
        return Error(__func__);
    }

    NvAPI_Status __cdecl NvAPI_Unload() {
        return Error(__func__);
    }


    NvAPI_Status __cdecl NvAPI_Initialize() {
        constexpr auto n = __func__;

        return NoLib(n);
    }

    NvAPI_Status __cdecl NvAPI_InitInterfaceEx(NvAPI_Status nr, NvAPI_ShortString szDesc) {
        return Error(__func__);
    }
}
