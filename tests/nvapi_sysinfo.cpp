using namespace trompeloeil;

TEST_CASE("GetInterfaceVersionString returns OK", "[.sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetInterfaceVersionString(desc) == NVAPI_OK);
    REQUIRE(strcmp(desc, "DXVK_NVAPI") == 0);
}

TEST_CASE("GetErrorMessage returns OK", "[.sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetErrorMessage(NVAPI_NVIDIA_DEVICE_NOT_FOUND, desc) == NVAPI_OK);
    REQUIRE(strcmp(desc, "NVAPI_NVIDIA_DEVICE_NOT_FOUND") == 0);
}

TEST_CASE("Initialize succeed", "[.sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    DXGIDxvkAdapterMock adapter;
    DXGIOutputMock output;

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vulkan, *nvml, adapter, output);

    SECTION("Initialize returns OK") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }

    SECTION("Initialize returns device-not-found when DXVK reports no adapters") {
        ALLOW_CALL(*dxgiFactory, EnumAdapters1(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(DXGI_ERROR_NOT_FOUND);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
        REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
    }

    SECTION("Initialize returns device-not-found when adapter with non NVIDIA driver ID has been found") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_MESA_RADV;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
        REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
    }
}

TEST_CASE("Topology methods succeed", "[.sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    DXGIDxvkAdapterMock adapter1;
    DXGIDxvkAdapterMock adapter2;
    DXGIOutputMock output1;
    DXGIOutputMock output2;
    DXGIOutputMock output3;

    auto e = ConfigureExtendedTestEnvironment(*dxgiFactory, *vulkan, *nvml, adapter1, adapter2, output1, output2, output3);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
    REQUIRE(NvAPI_Initialize() == NVAPI_OK);

    SECTION("EnumLogicalGPUs succeeds") {
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS]{};
        NvU32 count = 0U;

        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(handles[0] != handles[1]);
        REQUIRE(handles[0] != nullptr);
        REQUIRE(handles[1] != nullptr);
        REQUIRE(count == 2);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(reinterpret_cast<NvPhysicalGpuHandle>(handles[0]), name1) == NVAPI_OK);
        REQUIRE(strcmp(name1, "Device1") == 0);

        NvAPI_ShortString name2;
        REQUIRE(NvAPI_GPU_GetFullName(reinterpret_cast<NvPhysicalGpuHandle>(handles[1]), name2) == NVAPI_OK);
        REQUIRE(strcmp(name2, "Device2") == 0);
    }

    SECTION("EnumPhysicalGPUs succeeds") {
        NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS]{};
        NvU32 count = 0U;

        REQUIRE(NvAPI_EnumPhysicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(handles[0] != handles[1]);
        REQUIRE(handles[0] != nullptr);
        REQUIRE(handles[1] != nullptr);
        REQUIRE(count == 2);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(handles[0], name1) == NVAPI_OK);
        REQUIRE(strcmp(name1, "Device1") == 0);

        NvAPI_ShortString name2;
        REQUIRE(NvAPI_GPU_GetFullName(handles[1], name2) == NVAPI_OK);
        REQUIRE(strcmp(name2, "Device2") == 0);
    }

    SECTION("EnumEnumTCCPhysicalGPUs succeeds") {
        NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS];
        NvU32 count = -1U;
        REQUIRE(NvAPI_EnumTCCPhysicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(count == 0);
    }

    SECTION("EnumNvidiaDisplayHandle succeeds") {
        NvDisplayHandle handle1 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0U, &handle1) == NVAPI_OK);
        REQUIRE(handle1 != nullptr);

        NvDisplayHandle handle2 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(1U, &handle2) == NVAPI_OK);
        REQUIRE(handle2 != nullptr);
        REQUIRE(handle2 != handle1);

        NvDisplayHandle handle3 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(2U, &handle3) == NVAPI_OK);
        REQUIRE(handle3 != nullptr);
        REQUIRE(handle3 != handle1);
        REQUIRE(handle3 != handle2);

        NvDisplayHandle handle4 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(3U, &handle4) == NVAPI_END_ENUMERATION);
        REQUIRE(handle4 == nullptr);
    }

    SECTION("EnumNvidiaUnAttachedDisplayHandle succeeds") {
        NvUnAttachedDisplayHandle handle = nullptr;
        REQUIRE(NvAPI_EnumNvidiaUnAttachedDisplayHandle(0U, &handle) == NVAPI_END_ENUMERATION);
        REQUIRE(handle == nullptr);
    }

    SECTION("GetPhysicalGPUsFromLogicalGPU succeeds") {
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS];
        NvU32 count;
        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);

        NvPhysicalGpuHandle physicalHandles[NVAPI_MAX_PHYSICAL_GPUS];
        REQUIRE(NvAPI_GetPhysicalGPUsFromLogicalGPU(handles[0], physicalHandles, &count) == NVAPI_OK);
        REQUIRE(count == 1);
        REQUIRE(physicalHandles[0] == reinterpret_cast<NvPhysicalGpuHandle>(handles[0]));

        REQUIRE(NvAPI_GetPhysicalGPUsFromLogicalGPU(handles[1], physicalHandles, &count) == NVAPI_OK);
        REQUIRE(count == 1);
        REQUIRE(physicalHandles[0] == reinterpret_cast<NvPhysicalGpuHandle>(handles[1]));
    }

    SECTION("GetPhysicalGPUsFromDisplay succeeds") {
        NvDisplayHandle displayHandle1 = nullptr;
        NvDisplayHandle displayHandle2 = nullptr;
        NvDisplayHandle displayHandle3 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0U, &displayHandle1) == NVAPI_OK);
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(1U, &displayHandle2) == NVAPI_OK);
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(2U, &displayHandle3) == NVAPI_OK);

        NvPhysicalGpuHandle handles1[NVAPI_MAX_PHYSICAL_GPUS]{};
        NvU32 count1 = 0U;

        REQUIRE(NvAPI_GetPhysicalGPUsFromDisplay(displayHandle1, handles1, &count1) == NVAPI_OK);
        REQUIRE(handles1[0] != nullptr);
        REQUIRE(count1 == 1);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(handles1[0], name1) == NVAPI_OK);
        REQUIRE(strcmp(name1, "Device1") == 0);

        NvPhysicalGpuHandle handles2[NVAPI_MAX_PHYSICAL_GPUS]{};
        NvU32 count2 = 0;

        REQUIRE(NvAPI_GetPhysicalGPUsFromDisplay(displayHandle2, handles2, &count2) == NVAPI_OK);
        REQUIRE(handles2[0] != nullptr);
        REQUIRE(count2 == 1);
        REQUIRE(handles2[0] == handles1[0]);

        NvPhysicalGpuHandle handles3[NVAPI_MAX_PHYSICAL_GPUS]{};
        NvU32 count3 = 0U;

        REQUIRE(NvAPI_GetPhysicalGPUsFromDisplay(displayHandle3, handles3, &count3) == NVAPI_OK);
        REQUIRE(handles3[0] != nullptr);
        REQUIRE(count3 == 1);
        REQUIRE(handles3[0] != handles1[0]);

        NvAPI_ShortString name3;
        REQUIRE(NvAPI_GPU_GetFullName(handles3[0], name3) == NVAPI_OK);
        REQUIRE(strcmp(name3, "Device2") == 0);
    }

    SECTION("GetPhysicalGpuFromDisplayId succeeds") {
        NvPhysicalGpuHandle handle1 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0U, &handle1) == NVAPI_OK);
        REQUIRE(handle1 != nullptr);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(handle1, name1) == NVAPI_OK);
        REQUIRE(strcmp(name1, "Device1") == 0);

        NvPhysicalGpuHandle handle2 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(1U, &handle2) == NVAPI_OK);
        REQUIRE(handle2 != nullptr);
        REQUIRE(handle2 == handle1);

        NvPhysicalGpuHandle handle3 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(2U, &handle3) == NVAPI_OK);
        REQUIRE(handle3 != nullptr);

        NvAPI_ShortString name3;
        REQUIRE(NvAPI_GPU_GetFullName(handle3, name3) == NVAPI_OK);
        REQUIRE(strcmp(name3, "Device2") == 0);

        NvPhysicalGpuHandle handle4 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(3U, &handle4) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(handle4 == nullptr);
    }

    SECTION("GetAssociatedNvidiaDisplayName succeeds") {
        NvDisplayHandle handle1 = nullptr;
        NvDisplayHandle handle2 = nullptr;
        NvDisplayHandle handle3 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0U, &handle1) == NVAPI_OK);
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(1U, &handle2) == NVAPI_OK);
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(2U, &handle3) == NVAPI_OK);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GetAssociatedNvidiaDisplayName(handle1, name1) == NVAPI_OK);
        REQUIRE(strcmp(name1, "Output1") == 0);

        NvAPI_ShortString name2;
        REQUIRE(NvAPI_GetAssociatedNvidiaDisplayName(handle2, name2) == NVAPI_OK);
        REQUIRE(strcmp(name2, "Output2") == 0);

        NvAPI_ShortString name3;
        REQUIRE(NvAPI_GetAssociatedNvidiaDisplayName(handle3, name3) == NVAPI_OK);
        REQUIRE(strcmp(name3, "Output3") == 0);

        NvAPI_ShortString name4;
        NvDisplayHandle handle4 = nullptr;
        REQUIRE(NvAPI_GetAssociatedNvidiaDisplayName(handle4, name4) == NVAPI_INVALID_ARGUMENT);
    }
}

TEST_CASE("Sysinfo methods succeed", "[.sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    DXGIDxvkAdapterMock adapter;
    DXGIOutputMock output;

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vulkan, *nvml, adapter, output);

    ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", "");

    SECTION("Initialize and unloads return OK") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }

    SECTION("GetDriverAndBranchVersion returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        props->driverVersion = (470 << 22) | (35 << 14) | 1 << 6;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvU32 version;
        NvAPI_ShortString branch;
        REQUIRE(NvAPI_SYS_GetDriverAndBranchVersion(&version, branch) == NVAPI_OK);
        REQUIRE(version == 47035);
        REQUIRE(std::string(branch).length() > 0);
    }

    SECTION("GetDisplayDriverVersion returns OK") {
        struct Data {VkDriverId driverId; uint16_t major; uint16_t minor; uint16_t patch; uint32_t expectedVersion;};
        auto args = GENERATE(
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 470, 45, 1, 47045},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 470, 101, 1, 47099},
            Data{VK_DRIVER_ID_AMD_OPEN_SOURCE, 21, 2, 3, 2102});

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "1");

        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&args](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = args.driverId;
                        strcpy(props->deviceName, "GPU0");
                        if (args.driverId == VK_DRIVER_ID_NVIDIA_PROPRIETARY)
                            props->driverVersion = (args.major << 22) | (args.minor << 14) | (args.patch << 6);
                        else
                            props->driverVersion = (args.major << 22) | (args.minor << 12) | args.patch;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvDisplayHandle handle;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0, &handle) == NVAPI_OK);

        NV_DISPLAY_DRIVER_VERSION version;
        version.version = NV_DISPLAY_DRIVER_VERSION_VER;
        REQUIRE(NvAPI_GetDisplayDriverVersion(handle, &version) == NVAPI_OK);
        REQUIRE(version.drvVersion == args.expectedVersion);
        REQUIRE(strcmp(version.szAdapterString, "GPU0") == 0);
        REQUIRE(std::string(version.szBuildBranchString).length() > 0);

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "0");
    }

    SECTION("GetDisplayDriverVersion with version override returns OK") {
        struct Data {std::string override; uint32_t expectedVersion;};
        auto args = GENERATE(
            Data{"", 47045},
            Data{"0", 47045},
            Data{"99", 47045},
            Data{"100000", 47045},
            Data{"AB39976", 47045},
            Data{"39976AB", 47045},
            Data{"39976", 39976});

        ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", args.override.c_str());

        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        props->driverVersion = (470 << 22) | (45 << 14) | (0 << 6);
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvDisplayHandle handle;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0, &handle) == NVAPI_OK);

        NV_DISPLAY_DRIVER_VERSION version;
        version.version = NV_DISPLAY_DRIVER_VERSION_VER;
        REQUIRE(NvAPI_GetDisplayDriverVersion(handle, &version) == NVAPI_OK);
        REQUIRE(version.drvVersion == args.expectedVersion);

        ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", "");
    }

    SECTION("GetGPUIDFromPhysicalGPU / GetPhysicalGPUFromGPUID succeeds") {
        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        pciBusInfoProps->pciDomain = 0x01;
                        pciBusInfoProps->pciBus = 0x02;
                        pciBusInfoProps->pciDevice = 0x03;
                    })
            );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        SECTION("GetGPUIDFromPhysicalGPU succeeds") {
            NvU32 gpuId;
            REQUIRE(NvAPI_GetGPUIDfromPhysicalGPU(handle, &gpuId) == NVAPI_OK);
            REQUIRE(gpuId == 0x10203);
        }

        SECTION("GetPhysicalGPUFromGPUID succeeds") {
            NvPhysicalGpuHandle handleFromGpuId;
            REQUIRE(NvAPI_GetPhysicalGPUFromGPUID(0x10203, &handleFromGpuId) == NVAPI_OK);
            REQUIRE(handleFromGpuId == handle);
        }
    }

    SECTION("CudaEnumComputeCapableGpus returns OK") {
        struct Data {VkDriverId driverId; std::string extensionName; uint32_t gpuCount;};
        auto args = GENERATE(
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, 1},
            Data{VK_DRIVER_ID_MESA_RADV, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 0},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, "ext", 0});

        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [args](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = args.driverId;
                        if (args.extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                            fragmentShadingRateProps->primitiveFragmentShadingRateWithMultipleViewports = VK_TRUE;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NV_COMPUTE_GPU_TOPOLOGY gpuTopology;
        gpuTopology.version = NV_COMPUTE_GPU_TOPOLOGY_VER;
        gpuTopology.computeGpus = new NV_COMPUTE_GPU[1];
        REQUIRE(NvAPI_GPU_CudaEnumComputeCapableGpus(&gpuTopology) == NVAPI_OK);
        REQUIRE(gpuTopology.gpuCount == args.gpuCount);
        if (gpuTopology.gpuCount == 1) {
            REQUIRE(gpuTopology.computeGpus[0].hPhysicalGpu == handle);
            REQUIRE(gpuTopology.computeGpus[0].flags == 0x0b);
        }

        delete gpuTopology.computeGpus;
    }

    SECTION("GetGPUType returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        props->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NV_GPU_TYPE type;
        REQUIRE(NvAPI_GPU_GetGPUType(handle, &type) == NVAPI_OK);
        REQUIRE(type == NV_SYSTEM_TYPE_DGPU);
    }

    SECTION("GetPCIIdentifiers returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        props->vendorID = 0x10de;
                        props->deviceID = 0x1234;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvU32 deviceId, subSystemId, revisionId, extDeviceId;
        REQUIRE(NvAPI_GPU_GetPCIIdentifiers(handle, &deviceId, &subSystemId, &revisionId, &extDeviceId) == NVAPI_OK);
        REQUIRE(deviceId == 0x123410de);
        REQUIRE(subSystemId == 0x0);
        REQUIRE(revisionId == 0x0);
        REQUIRE(extDeviceId == 0x00001234);
    }

    SECTION("GetFullName returns OK") {
        auto name = "High-End GPU01";
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&name](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        strcpy(props->deviceName, name);
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvAPI_ShortString fullName;
        REQUIRE(NvAPI_GPU_GetFullName(handle, fullName) == NVAPI_OK);
        REQUIRE(strcmp(fullName, name) == 0);
    }

    SECTION("GetBusId returns OK") {
        auto id = 2U;
        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&id](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        pciBusInfoProps->pciBus = id;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvU32 busId;
        REQUIRE(NvAPI_GPU_GetBusId(handle, &busId) == NVAPI_OK);
        REQUIRE(busId == id);
    }

    SECTION("GetBusSlotId returns OK") {
        auto id = 3U;
        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&id](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        pciBusInfoProps->pciDevice = id;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvU32 busSlotId;
        REQUIRE(NvAPI_GPU_GetBusSlotId(handle, &busSlotId) == NVAPI_OK);
        REQUIRE(busSlotId == id);
    }

    SECTION("GetBusType returns OK") {
        struct Data {std::string extensionName; NV_GPU_BUS_TYPE expectedBusType;};
        auto args = GENERATE(
            Data{VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, NVAPI_GPU_BUS_TYPE_PCI_EXPRESS},
            Data{"ext", NVAPI_GPU_BUS_TYPE_UNDEFINED});

        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NV_GPU_BUS_TYPE type;
        REQUIRE(NvAPI_GPU_GetBusType(handle, &type) == NVAPI_OK);
        REQUIRE(type == args.expectedBusType);
    }

    SECTION("GetPhysicalFrameBufferSize returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceMemoryProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT({
                _3->memoryProperties.memoryHeapCount = 1;
                _3->memoryProperties.memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
                _3->memoryProperties.memoryHeaps[0].size = 8191 * 1024;
            });

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvU32 size;
        REQUIRE(NvAPI_GPU_GetPhysicalFrameBufferSize(handle, &size) == NVAPI_OK);
        REQUIRE(size == 8191);
    }

    SECTION("GetAdapterIdFromPhysicalGpu returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                 ConfigureGetPhysicalDeviceProperties2(_3,
                     [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                         auto luid = LUID{0x04030211, 0x08070655};
                         memcpy(&idProps->deviceLUID, &luid, sizeof(luid));
                         idProps->deviceLUIDValid = VK_TRUE;
                         driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                     }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        LUID luid;
        REQUIRE(NvAPI_GPU_GetAdapterIdFromPhysicalGpu(handle, static_cast<void*>(&luid)) == NVAPI_OK);
        REQUIRE(luid.HighPart == 0x08070655);
        REQUIRE(luid.LowPart  == 0x04030211);
    }

    SECTION("GetArchInfo returns OK") {
        struct Data {std::string extensionName; NV_GPU_ARCHITECTURE_ID expectedArchId; NV_GPU_ARCH_IMPLEMENTATION_ID expectedImplId;};
        auto args = GENERATE(
            Data{VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GA100, NV_GPU_ARCH_IMPLEMENTATION_GA102},
            Data{VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME, NV_GPU_ARCHITECTURE_TU100, NV_GPU_ARCH_IMPLEMENTATION_TU102},
            Data{VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GV100, NV_GPU_ARCH_IMPLEMENTATION_GV100},
            Data{VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GP100, NV_GPU_ARCH_IMPLEMENTATION_GP102},
            Data{VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GM200, NV_GPU_ARCH_IMPLEMENTATION_GM204},
            Data{"ext", NV_GPU_ARCHITECTURE_GK100, NV_GPU_ARCH_IMPLEMENTATION_GK104});

        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&args](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        if (args.extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                            fragmentShadingRateProps->primitiveFragmentShadingRateWithMultipleViewports = VK_TRUE;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER_2;
        REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) == NVAPI_OK);
        REQUIRE(archInfo.architecture_id == args.expectedArchId);
        REQUIRE(archInfo.implementation_id == args.expectedImplId);
        REQUIRE(archInfo.revision_id == NV_GPU_CHIP_REV_A01);
    }

    SECTION("GetArchInfo returns device-not-found when no NVIDIA device is present") {
        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "1");

        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_MESA_RADV;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER_2;
        REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) == NVAPI_NVIDIA_DEVICE_NOT_FOUND);

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "0");
    }

    SECTION("NVML depending methods succeed when NVML is available") {
        ALLOW_CALL(*nvml, IsAvailable()) // NOLINT(bugprone-use-after-move)
            .RETURN(true);
        ALLOW_CALL(*nvml, DeviceGetHandleByPciBusId_v2(_, _))
            .RETURN(NVML_SUCCESS);

        SECTION("GetPCIIdentifiers returns OK and has subsystem ID when NVML is available") {
            auto id = 0x88161043;
            ALLOW_CALL(*nvml, DeviceGetPciInfo_v3(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(_2->pciSubSystemId = id)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

            NvU32 deviceId, subSystemId, revisionId, extDeviceId;
            REQUIRE(NvAPI_GPU_GetPCIIdentifiers(handle, &deviceId, &subSystemId, &revisionId, &extDeviceId) == NVAPI_OK);
            REQUIRE(subSystemId == id);
        }

        SECTION("GetVbiosVersionString returns OK") {
            auto version = "12.34";
            ALLOW_CALL(*nvml, DeviceGetVbiosVersion(_, _, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(strcpy(_2, version))
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

            NvAPI_ShortString revision;
            REQUIRE(NvAPI_GPU_GetVbiosVersionString(handle, revision) == NVAPI_OK);
            REQUIRE(strcmp(revision, version) == 0);
        }

        SECTION("GetDynamicPstatesInfoEx returns OK") {
            auto gpuUtilization = 32U;
            auto memoryUtilization = 56U;
            ALLOW_CALL(*nvml, DeviceGetUtilizationRates(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT({
                    _2->gpu = gpuUtilization;
                    _2->memory = memoryUtilization;
                })
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

            NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
            info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
            REQUIRE(NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info) == NVAPI_OK);
            REQUIRE(info.flags == 0);
            REQUIRE(info.utilization[0].bIsPresent == 1);
            REQUIRE(info.utilization[0].percentage == gpuUtilization);
            REQUIRE(info.utilization[1].bIsPresent == 1);
            REQUIRE(info.utilization[1].percentage == memoryUtilization);
            REQUIRE(info.utilization[2].bIsPresent == 1);
            REQUIRE(info.utilization[2].percentage == 0);
            REQUIRE(info.utilization[3].bIsPresent == 1);
            REQUIRE(info.utilization[3].percentage == 0);
            for (auto i = 4U; i < NVAPI_MAX_GPU_UTILIZATIONS; i++)
                REQUIRE(info.utilization[i].bIsPresent == 0);
        }

        SECTION("GetThermalSettings returns OK") {
            auto temp = 65;
            ALLOW_CALL(*nvml, DeviceGetTemperature(_, _, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_3 = temp)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

            NV_GPU_THERMAL_SETTINGS settings;
            settings.version = NV_GPU_THERMAL_SETTINGS_VER_2;
            REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_OK);
            REQUIRE(settings.count == 1);
            REQUIRE(settings.sensor[0].controller == NVAPI_THERMAL_CONTROLLER_UNKNOWN);
            REQUIRE(settings.sensor[0].target == NVAPI_THERMAL_TARGET_GPU);
            REQUIRE(settings.sensor[0].currentTemp == static_cast<int>(temp));
            REQUIRE(settings.sensor[0].defaultMaxTemp == 127);
            REQUIRE(settings.sensor[0].defaultMinTemp == -256);
        }

        SECTION("GetCurrentPstate returns OK") {
            ALLOW_CALL(*nvml, DeviceGetPerformanceState(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_2 = NVML_PSTATE_2)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

            NV_GPU_PERF_PSTATE_ID pstate;
            REQUIRE(NvAPI_GPU_GetCurrentPstate(handle, &pstate) == NVAPI_OK);
            REQUIRE(pstate == NVAPI_GPU_PERF_PSTATE_P2);
        }

        SECTION("GetAllClockFrequencies returns OK") {
            auto graphicsClock = 500U;
            auto memoryClock = 600U;
            auto videoClock = 700U;
            ALLOW_CALL(*nvml, DeviceGetClockInfo(_, NVML_CLOCK_GRAPHICS, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_3 = graphicsClock)
                .RETURN(NVML_SUCCESS);
            ALLOW_CALL(*nvml, DeviceGetClockInfo(_, NVML_CLOCK_MEM, _))
                .LR_SIDE_EFFECT(*_3 = memoryClock)
                .RETURN(NVML_SUCCESS);
            ALLOW_CALL(*nvml, DeviceGetClockInfo(_, NVML_CLOCK_VIDEO, _))
                .LR_SIDE_EFFECT(*_3 = videoClock)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

            NV_GPU_CLOCK_FREQUENCIES frequencies;
            frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
            frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
            REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_OK);
            REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent == 1);
            REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency == graphicsClock * 1000);
            REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent == 1);
            REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency == memoryClock * 1000);
            REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].bIsPresent == 1);
            REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].frequency == videoClock * 1000);
        }
    }

    SECTION("NVML depending methods succeed when NVML is not available") {
        ALLOW_CALL(*nvml, IsAvailable()) // NOLINT(bugprone-use-after-move)
            .RETURN(false);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        SECTION("GetVbiosVersionString returns OK when NVML is not available") {
            NvAPI_ShortString revision;
            REQUIRE(NvAPI_GPU_GetVbiosVersionString(handle, revision) == NVAPI_OK);
            REQUIRE(strcmp(revision, "N/A") == 0);
        }

        SECTION("NVML depending methods return no-implementation when NVML is not available") {
            NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
            info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
            REQUIRE(NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info) == NVAPI_NO_IMPLEMENTATION);
            NV_GPU_THERMAL_SETTINGS settings;
            settings.version = NV_GPU_THERMAL_SETTINGS_VER_2;
            REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_NO_IMPLEMENTATION);
            NV_GPU_PERF_PSTATE_ID pstate;
            REQUIRE(NvAPI_GPU_GetCurrentPstate(handle, &pstate) == NVAPI_NO_IMPLEMENTATION);
            NV_GPU_CLOCK_FREQUENCIES frequencies;
            frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
            frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
            REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_NO_IMPLEMENTATION);
        }
    }

    SECTION("NVML depending methods succeed when NVML is available but without suitable adapter") {
        ALLOW_CALL(*nvml, IsAvailable()) // NOLINT(bugprone-use-after-move)
            .RETURN(true);
        ALLOW_CALL(*nvml, DeviceGetHandleByPciBusId_v2(_, _))
            .RETURN(NVML_ERROR_NOT_FOUND);
        ALLOW_CALL(*nvml, ErrorString(_))
            .RETURN("error");

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        SECTION("GetVbiosVersionString returns OK when NVML is available but without suitable adapter") {
            NvAPI_ShortString revision;
            REQUIRE(NvAPI_GPU_GetVbiosVersionString(handle, revision) == NVAPI_OK);
            REQUIRE(strcmp(revision, "N/A") == 0);
        }

        SECTION("NVML depending methods return handle-invalidated when NVML is available but without suitable adapter") {
            NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
            info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
            REQUIRE(NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info) == NVAPI_HANDLE_INVALIDATED);
            NV_GPU_THERMAL_SETTINGS settings;
            settings.version = NV_GPU_THERMAL_SETTINGS_VER_2;
            REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_HANDLE_INVALIDATED);
            NV_GPU_PERF_PSTATE_ID pstate;
            REQUIRE(NvAPI_GPU_GetCurrentPstate(handle, &pstate) == NVAPI_HANDLE_INVALIDATED);
            NV_GPU_CLOCK_FREQUENCIES frequencies;
            frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
            frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
            REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_HANDLE_INVALIDATED);
        }
    }
}

TEST_CASE("GetHdrCapabilities returns OK", "[.sysinfo]") {
    NV_HDR_CAPABILITIES capabilities;
    capabilities.version = NV_HDR_CAPABILITIES_VER2;
    REQUIRE(NvAPI_Disp_GetHdrCapabilities(0, &capabilities) == NVAPI_OK);
    REQUIRE(capabilities.isST2084EotfSupported == false);
    REQUIRE(capabilities.isTraditionalHdrGammaSupported == false);
    REQUIRE(capabilities.isEdrSupported == false);
    REQUIRE(capabilities.driverExpandDefaultHdrParameters == false);
    REQUIRE(capabilities.isTraditionalSdrGammaSupported == false);
    REQUIRE(capabilities.isDolbyVisionSupported == false);
}

TEST_CASE("GetDisplayViewportsByResolution returns mosaic-not-active", "[.sysinfo]") {
    NvU8 corrected;
    NV_RECT rect[NV_MOSAIC_MAX_DISPLAYS];
    REQUIRE(NvAPI_Mosaic_GetDisplayViewportsByResolution(0, 0, 0, rect, &corrected) == NVAPI_MOSAIC_NOT_ACTIVE);
}
