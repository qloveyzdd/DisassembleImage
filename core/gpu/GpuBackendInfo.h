#pragma once

#include <string>

#include "../model/ProcessingTask.h"

namespace disassemble::core {

struct GpuBackendInfo {
    bool openClLoaderPresent = false;
    bool openClSupported = false;
    bool platformEnumerationSucceeded = false;
    bool gpuDeviceAvailable = false;
    int platformCount = 0;
    int gpuDeviceCount = 0;
    std::string deviceName;
    std::string vendorName;
    std::string driverVersion;
    std::string openClVersion;
    std::string selectedPlatformName;
    std::string loaderStatus;
    std::string platformSummary;
    std::string deviceEnumerationStatus;
    std::string contextStatus;
    std::string failureStage;
    std::string recommendation;
    std::string unavailableReason;

    bool canUseGpu() const;
};

GpuBackendInfo detectGpuBackend();
ProcessingBackend resolveActiveBackend(ProcessingBackend requestedBackend,
                                       const GpuBackendInfo &backendInfo,
                                       std::string &fallbackReason);
std::string processingBackendLabel(ProcessingBackend backend);

} // namespace disassemble::core
