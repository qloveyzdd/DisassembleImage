#pragma once

#include <string>

#include "../model/ProcessingTask.h"

namespace disassemble::core {

struct GpuBackendInfo {
    bool openClSupported = false;
    bool gpuDeviceAvailable = false;
    std::string deviceName;
    std::string unavailableReason;

    bool canUseGpu() const;
};

GpuBackendInfo detectGpuBackend();
ProcessingBackend resolveActiveBackend(ProcessingBackend requestedBackend,
                                       const GpuBackendInfo &backendInfo,
                                       std::string &fallbackReason);
std::string processingBackendLabel(ProcessingBackend backend);

} // namespace disassemble::core
