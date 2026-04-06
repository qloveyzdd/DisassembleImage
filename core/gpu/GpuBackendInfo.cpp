#include "GpuBackendInfo.h"

#include <sstream>
#include <vector>

#include <opencv2/core/ocl.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace disassemble::core {

namespace {

bool hasOpenClLoader()
{
#ifdef _WIN32
    HMODULE module = LoadLibraryW(L"OpenCL.dll");
    if (module == nullptr) {
        return false;
    }
    FreeLibrary(module);
    return true;
#else
    return cv::ocl::haveOpenCL();
#endif
}

std::string joinLines(const std::vector<std::string> &lines)
{
    std::ostringstream stream;
    for (size_t index = 0; index < lines.size(); ++index) {
        if (index > 0) {
            stream << "\n";
        }
        stream << lines[index];
    }
    return stream.str();
}

std::string defaultRecommendation(const std::string &failureStage)
{
    if (failureStage == "loader") {
        return "Install or reinstall a full GPU driver with OpenCL support.";
    }
    if (failureStage == "icd_platform_enum") {
        return "Check OpenCL platform registration and reinstall the GPU driver if needed.";
    }
    if (failureStage == "device_enum") {
        return "Make sure the current driver exposes an OpenCL GPU device.";
    }
    if (failureStage == "context_create") {
        return "Update the driver and retry, or continue with CPU mode.";
    }
    return "The current environment is ready for GPU execution.";
}

} // namespace

bool GpuBackendInfo::canUseGpu() const
{
    return openClSupported && gpuDeviceAvailable;
}

GpuBackendInfo detectGpuBackend()
{
    GpuBackendInfo info;
    info.openClLoaderPresent = hasOpenClLoader();
    info.loaderStatus = info.openClLoaderPresent
        ? "OpenCL.dll found"
        : "OpenCL.dll not found";

    if (!info.openClLoaderPresent) {
        info.failureStage = "loader";
        info.unavailableReason = "OpenCL loader was not found in the system.";
        info.deviceEnumerationStatus = "device enumeration not started";
        info.contextStatus = "GPU context not created";
        info.recommendation = defaultRecommendation(info.failureStage);
        return info;
    }

    std::vector<cv::ocl::PlatformInfo> platforms;
    try {
        cv::ocl::getPlatfomsInfo(platforms);
        info.platformEnumerationSucceeded = true;
        info.platformCount = static_cast<int>(platforms.size());

        std::vector<std::string> platformLines;
        platformLines.reserve(platforms.size());
        for (const auto &platform : platforms) {
            platformLines.push_back(platform.name() + " | " + platform.vendor() + " | " + platform.version());
        }
        info.platformSummary = platformLines.empty() ? "no OpenCL platform enumerated" : joinLines(platformLines);
    } catch (const std::exception &error) {
        info.platformEnumerationSucceeded = false;
        info.platformSummary = std::string("platform enumeration exception: ") + error.what();
    }

    info.openClSupported = cv::ocl::haveOpenCL();
    if (!info.openClSupported) {
        info.failureStage = "icd_platform_enum";
        if (!info.platformEnumerationSucceeded) {
            info.unavailableReason = "OpenCV failed to enumerate OpenCL platforms.";
        } else if (info.platformCount == 0) {
            info.unavailableReason = "No OpenCL platform was enumerated. ICD registration or driver installation is likely incomplete.";
        } else {
            info.unavailableReason = "OpenCL platforms were found, but OpenCV still reports OpenCL as unavailable.";
        }
        info.deviceEnumerationStatus = "device enumeration not started";
        info.contextStatus = "GPU context not created";
        info.recommendation = defaultRecommendation(info.failureStage);
        return info;
    }

    std::vector<std::string> gpuDeviceLines;
    for (const auto &platform : platforms) {
        for (int index = 0; index < platform.deviceNumber(); ++index) {
            cv::ocl::Device device;
            platform.getDevice(device, index);
            if (!device.available()) {
                continue;
            }
            if ((device.type() & cv::ocl::Device::TYPE_GPU) == 0) {
                continue;
            }

            ++info.gpuDeviceCount;
            if (info.selectedPlatformName.empty()) {
                info.selectedPlatformName = platform.name();
            }
            gpuDeviceLines.push_back(platform.name() + " -> " + device.name());
        }
    }
    info.deviceEnumerationStatus = gpuDeviceLines.empty()
        ? "no available OpenCL GPU device found on the enumerated platforms"
        : joinLines(gpuDeviceLines);

    if (info.gpuDeviceCount == 0) {
        info.failureStage = "device_enum";
        info.unavailableReason = "OpenCL platforms were found, but no available GPU device was exposed.";
        info.contextStatus = "GPU context not created";
        info.recommendation = defaultRecommendation(info.failureStage);
        return info;
    }

    cv::ocl::Context context;
    if (!context.create(cv::ocl::Device::TYPE_GPU) || context.ndevices() <= 0) {
        info.failureStage = "context_create";
        info.unavailableReason = "GPU devices were found, but OpenCL GPU context creation failed.";
        info.contextStatus = "GPU context creation failed";
        info.recommendation = defaultRecommendation(info.failureStage);
        return info;
    }

    info.gpuDeviceAvailable = true;
    info.contextStatus = "GPU context created";
    const cv::ocl::Device device = context.device(0);
    info.deviceName = device.name();
    if (info.deviceName.empty()) {
        info.deviceName = "OpenCL GPU";
    }
    info.vendorName = device.vendorName();
    info.driverVersion = device.driverVersion();
    info.openClVersion = device.OpenCL_C_Version();
    if (info.selectedPlatformName.empty()) {
        info.selectedPlatformName = "OpenCL platform";
    }
    info.failureStage = "ready";
    info.recommendation = defaultRecommendation(info.failureStage);
    return info;
}

ProcessingBackend resolveActiveBackend(ProcessingBackend requestedBackend,
                                       const GpuBackendInfo &backendInfo,
                                       std::string &fallbackReason)
{
    fallbackReason.clear();

    if (requestedBackend == ProcessingBackend::Cpu) {
        return ProcessingBackend::Cpu;
    }

    if (backendInfo.canUseGpu()) {
        return ProcessingBackend::Gpu;
    }

    if (requestedBackend == ProcessingBackend::Gpu) {
        fallbackReason = std::string("GPU was requested but is unavailable, falling back to CPU: ")
            + backendInfo.unavailableReason;
    } else {
        fallbackReason = std::string("Auto mode did not find a usable GPU, falling back to CPU: ")
            + backendInfo.unavailableReason;
    }
    return ProcessingBackend::Cpu;
}

std::string processingBackendLabel(ProcessingBackend backend)
{
    switch (backend) {
    case ProcessingBackend::Auto:
        return "Auto";
    case ProcessingBackend::Cpu:
        return "CPU";
    case ProcessingBackend::Gpu:
        return "GPU";
    }
    return "Auto";
}

} // namespace disassemble::core
