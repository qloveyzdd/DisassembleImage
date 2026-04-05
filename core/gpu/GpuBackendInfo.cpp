#include "GpuBackendInfo.h"

#include <opencv2/core/ocl.hpp>

namespace disassemble::core {

bool GpuBackendInfo::canUseGpu() const
{
    return openClSupported && gpuDeviceAvailable;
}

GpuBackendInfo detectGpuBackend()
{
    GpuBackendInfo info;
    info.openClSupported = cv::ocl::haveOpenCL();
    if (!info.openClSupported) {
        info.unavailableReason = u8"当前 OpenCV 或驱动环境没有可用的 OpenCL 支持";
        return info;
    }

    cv::ocl::Context context;
    if (!context.create(cv::ocl::Device::TYPE_GPU) || context.ndevices() <= 0) {
        info.unavailableReason = u8"没有检测到可用的 OpenCL GPU 设备";
        return info;
    }

    info.gpuDeviceAvailable = true;
    info.deviceName = context.device(0).name();
    if (info.deviceName.empty()) {
        info.deviceName = u8"OpenCL GPU";
    }
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
        fallbackReason = std::string(u8"未能启用 GPU，已回退到 CPU：") + backendInfo.unavailableReason;
    } else {
        fallbackReason = std::string(u8"自动模式未发现可用 GPU，已改用 CPU：") + backendInfo.unavailableReason;
    }
    return ProcessingBackend::Cpu;
}

std::string processingBackendLabel(ProcessingBackend backend)
{
    switch (backend) {
    case ProcessingBackend::Auto:
        return u8"自动";
    case ProcessingBackend::Cpu:
        return "CPU";
    case ProcessingBackend::Gpu:
        return "GPU";
    }
    return u8"自动";
}

} // namespace disassemble::core
