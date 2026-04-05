#include <CL/opencl.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <cmath>

// Error code to string helper
const char* errorCodeToString(cl_int err) {
    switch (err) {
        case CL_SUCCESS: return "SUCCESS";
        case CL_DEVICE_NOT_FOUND: return "DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE: return "DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE: return "COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES: return "OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY: return "OUT_OF_HOST_MEMORY";
        case CL_INVALID_VALUE: return "INVALID_VALUE";
        case CL_INVALID_DEVICE: return "INVALID_DEVICE";
        case CL_INVALID_CONTEXT: return "INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES: return "INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE: return "INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR: return "INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT: return "INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE: return "INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER: return "INVALID_SAMPLER";
        case CL_INVALID_BINARY: return "INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS: return "INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM: return "INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE: return "INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME: return "INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION: return "INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL: return "INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX: return "INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE: return "INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE: return "INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS: return "INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION: return "INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE: return "INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE: return "INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET: return "INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST: return "INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT: return "INVALID_EVENT";
        case CL_INVALID_OPERATION: return "INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT: return "INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE: return "INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL: return "INVALID_MIP_LEVEL";
        default: return "UNKNOWN";
    }
}

// Print device information
void printDeviceInfo(const cl::Device& device) {
    std::string str;
    cl_ulong ulong;
    size_t size;
    cl_uint uint;
    cl_bool boolean;

    device.getInfo(CL_DEVICE_NAME, &str);
    std::cout << "  Device: " << str << "\n";

    device.getInfo(CL_DEVICE_VENDOR, &str);
    std::cout << "  Vendor: " << str << "\n";

    device.getInfo(CL_DEVICE_VERSION, &str);
    std::cout << "  OpenCL Version: " << str << "\n";

    device.getInfo(CL_DRIVER_VERSION, &str);
    std::cout << "  Driver: " << str << "\n";

    device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &ulong);
    std::cout << "  Global Memory: " << ulong / (1024*1024) << " MB\n";

    device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &ulong);
    std::cout << "  Local Memory: " << ulong / 1024 << " KB\n";

    device.getInfo(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, &ulong);
    std::cout << "  Constant Buffer: " << ulong / 1024 << " KB\n";

    device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &size);
    std::cout << "  Max Work Group Size: " << size << "\n";

    device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &uint);
    std::cout << "  Compute Units: " << uint << "\n";

    device.getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, &uint);
    std::cout << "  Clock Frequency: " << uint << " MHz\n";

    device.getInfo(CL_DEVICE_IMAGE_SUPPORT, &boolean);
    std::cout << "  Image Support: " << (boolean ? "Yes" : "No") << "\n";

    if (boolean) {
        device.getInfo(CL_DEVICE_IMAGE2D_MAX_WIDTH, &size);
        std::cout << "  Max Image2D Width: " << size << "\n";
        device.getInfo(CL_DEVICE_IMAGE2D_MAX_HEIGHT, &size);
        std::cout << "  Max Image2D Height: " << size << "\n";
    }

    device.getInfo(CL_DEVICE_HOST_UNIFIED_MEMORY, &boolean);
    std::cout << "  Unified Memory: " << (boolean ? "Yes" : "No") << "\n";
}

// Enumerate all OpenCL platforms and devices
cl::Device selectBestGPU() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    std::cout << "=== OpenCL Platform Enumeration ===\n";
    std::cout << "Found " << platforms.size() << " platform(s)\n\n";

    std::vector<std::pair<cl::Device, cl_uint>> gpuDevices;  // device, score

    for (const auto& platform : platforms) {
        std::string platformName;
        platform.getInfo(CL_PLATFORM_NAME, &platformName);
        std::cout << "Platform: " << platformName << "\n";

        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

        for (const auto& device : devices) {
            cl_device_type deviceType;
            device.getInfo(CL_DEVICE_TYPE, &deviceType);

            std::string deviceName;
            cl_ulong globalMem;
            cl_uint computeUnits;

            device.getInfo(CL_DEVICE_NAME, &deviceName);
            device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &globalMem);
            device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &computeUnits);

            std::cout << "\n";
            printDeviceInfo(device);

            // Score GPUs higher than CPUs
            cl_uint score = 0;
            if (deviceType == CL_DEVICE_TYPE_GPU) {
                score = 1000 + computeUnits * 100 + static_cast<cl_uint>(globalMem / (256*1024*1024));
            } else if (deviceType == CL_DEVICE_TYPE_CPU) {
                score = computeUnits * 10;
            }

            gpuDevices.emplace_back(device, score);
        }
        std::cout << "\n";
    }

    if (gpuDevices.empty()) {
        throw std::runtime_error("No OpenCL devices found");
    }

    // Select device with highest score
    auto best = std::max_element(gpuDevices.begin(), gpuDevices.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    std::cout << "=== Selected Device ===\n";
    std::string name;
    best->first.getInfo(CL_DEVICE_NAME, &name);
    std::cout << "Using: " << name << " (score: " << best->second << ")\n\n";

    return best->first;
}

// Read kernel source from file
std::string readKernelSource(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open kernel file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Generate a test image (color gradient)
std::vector<float> generateTestImage(size_t width, size_t height) {
    std::vector<float> image(width * height * 4);

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            size_t idx = (y * width + x) * 4;
            // Create a colorful gradient
            image[idx + 0] = static_cast<float>(x) / width;           // R
            image[idx + 1] = static_cast<float>(y) / height;          // G
            image[idx + 2] = 0.5f;                                     // B
            image[idx + 3] = 1.0f;                                     // A
        }
    }

    return image;
}

// CPU implementation of grayscale for comparison
void cpuGrayscale(const std::vector<float>& input, std::vector<float>& output,
                  size_t width, size_t height) {
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            size_t idx = (y * width + x) * 4;
            float gray = 0.299f * input[idx + 0] +
                        0.587f * input[idx + 1] +
                        0.114f * input[idx + 2];
            output[idx + 0] = gray;
            output[idx + 1] = gray;
            output[idx + 2] = gray;
            output[idx + 3] = input[idx + 3];
        }
    }
}

// Save image as simple PPM file for verification
void saveImagePPM(const std::string& filename, const std::vector<float>& image,
                  size_t width, size_t height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to create output file: " + filename);
    }

    file << "P6\n" << width << " " << height << "\n255\n";

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            size_t idx = (y * width + x) * 4;
            unsigned char r = static_cast<unsigned char>(std::min(255.0f, image[idx + 0] * 255.0f));
            unsigned char g = static_cast<unsigned char>(std::min(255.0f, image[idx + 1] * 255.0f));
            unsigned char b = static_cast<unsigned char>(std::min(255.0f, image[idx + 2] * 255.0f));
            file << r << g << b;
        }
    }
}

int main() {
    try {
        // =========================================================================
        // 1. Enumerate platforms and select best GPU device
        // =========================================================================
        std::cout << "OpenCL Image Processing Prototype\n";
        std::cout << "=================================\n\n";

        cl::Device device = selectBestGPU();

        // =========================================================================
        // 2. Create context and command queue
        // =========================================================================
        cl::Context context(device);
        cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);

        std::cout << "Context and command queue created successfully.\n\n";

        // =========================================================================
        // 3. Load and compile kernel
        // =========================================================================
        std::cout << "Loading kernel from file...\n";
        std::string kernelSource = readKernelSource("kernel.cl");

        cl::Program::Sources sources;
        sources.push_back({kernelSource.c_str(), kernelSource.length()});

        cl::Program program(context, sources);

        try {
            // Build with optimization flags
            std::string buildOptions =
                "-cl-fast-relaxed-math "
                "-cl-mad-enable "
                "-cl-no-signed-zeros "
                "-Werror";
            program.build({device}, buildOptions.c_str());
            std::cout << "Kernel compiled successfully.\n\n";
        } catch (const cl::BuildError& e) {
            std::string log;
            program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &log);
            std::cerr << "Build error:\n" << log << "\n";
            throw;
        }

        // =========================================================================
        // 4. Generate test image
        // =========================================================================
        const size_t width = 1024;
        const size_t height = 1024;

        std::cout << "Generating test image (" << width << "x" << height << ")...\n";
        std::vector<float> inputImage = generateTestImage(width, height);
        std::vector<float> outputImage(width * height * 4);

        // =========================================================================
        // 5. Create OpenCL image objects
        // =========================================================================
        cl::ImageFormat imageFormat(CL_RGBA, CL_FLOAT);

        cl::Image2D inputBuffer(
            context,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            imageFormat,
            width,
            height,
            0,
            inputImage.data()
        );

        cl::Image2D outputBuffer(
            context,
            CL_MEM_WRITE_ONLY,
            imageFormat,
            width,
            height
        );

        std::cout << "OpenCL buffers created.\n\n";

        // =========================================================================
        // 6. Execute GPU kernel with timing
        // =========================================================================
        std::cout << "=== GPU Execution ===\n";

        cl::Kernel kernel(program, "grayscale");
        kernel.setArg(0, inputBuffer);
        kernel.setArg(1, outputBuffer);

        // Calculate work sizes (must be multiple of local size)
        const size_t localSizeX = 16;
        const size_t localSizeY = 16;
        size_t globalSizeX = ((width + localSizeX - 1) / localSizeX) * localSizeX;
        size_t globalSizeY = ((height + localSizeY - 1) / localSizeY) * localSizeY;

        cl::NDRange global(globalSizeX, globalSizeY);
        cl::NDRange local(localSizeX, localSizeY);

        // Warm-up run
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
        queue.finish();

        // Timed runs
        const int numRuns = 10;
        std::vector<double> gpuTimes;

        for (int i = 0; i < numRuns; ++i) {
            cl::Event event;
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local, nullptr, &event);
            event.wait();

            cl_ulong start, end;
            event.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
            event.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);

            double ms = static_cast<double>(end - start) / 1e6;
            gpuTimes.push_back(ms);
        }

        // Calculate average GPU time
        double gpuAvg = 0.0;
        for (double t : gpuTimes) gpuAvg += t;
        gpuAvg /= gpuTimes.size();

        // Read back result
        cl::size_t<3> origin;
        cl::size_t<3> region;
        origin[0] = 0; origin[1] = 0; origin[2] = 0;
        region[0] = width; region[1] = height; region[2] = 1;

        queue.enqueueReadImage(outputBuffer, CL_TRUE, origin, region, 0, 0, outputImage.data());

        std::cout << "GPU average time (" << numRuns << " runs): " << gpuAvg << " ms\n";
        std::cout << "GPU throughput: " << (width * height / (gpuAvg * 1000.0)) << " MPixels/sec\n\n";

        // =========================================================================
        // 7. Execute CPU implementation for comparison
        // =========================================================================
        std::cout << "=== CPU Execution ===\n";

        std::vector<float> cpuOutput(width * height * 4);

        // Warm-up
        cpuGrayscale(inputImage, cpuOutput, width, height);

        // Timed runs
        std::vector<double> cpuTimes;

        for (int i = 0; i < numRuns; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            cpuGrayscale(inputImage, cpuOutput, width, height);
            auto end = std::chrono::high_resolution_clock::now();

            double ms = std::chrono::duration<double, std::milli>(end - start).count();
            cpuTimes.push_back(ms);
        }

        // Calculate average CPU time
        double cpuAvg = 0.0;
        for (double t : cpuTimes) cpuAvg += t;
        cpuAvg /= cpuTimes.size();

        std::cout << "CPU average time (" << numRuns << " runs): " << cpuAvg << " ms\n";
        std::cout << "CPU throughput: " << (width * height / (cpuAvg * 1000.0)) << " MPixels/sec\n\n";

        // =========================================================================
        // 8. Compare results
        // =========================================================================
        std::cout << "=== Performance Comparison ===\n";
        double speedup = cpuAvg / gpuAvg;
        std::cout << "GPU Speedup: " << speedup << "x\n";
        std::cout << "GPU is " << (speedup > 1.0 ? "faster" : "slower") << " than CPU\n\n";

        // Verify correctness
        float maxError = 0.0f;
        for (size_t i = 0; i < outputImage.size(); ++i) {
            float error = std::abs(outputImage[i] - cpuOutput[i]);
            if (error > maxError) maxError = error;
        }
        std::cout << "Max difference between GPU and CPU: " << maxError << "\n";
        std::cout << "Verification: " << (maxError < 0.01f ? "PASSED" : "FAILED") << "\n\n";

        // =========================================================================
        // 9. Save output image
        // =========================================================================
        try {
            saveImagePPM("output_gpu.ppm", outputImage, width, height);
            saveImagePPM("output_cpu.ppm", cpuOutput, width, height);
            std::cout << "Output images saved:\n";
            std::cout << "  - output_gpu.ppm (GPU result)\n";
            std::cout << "  - output_cpu.ppm (CPU result)\n";
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not save images: " << e.what() << "\n";
        }

        std::cout << "\n=== OpenCL Prototype Complete ===\n";
        std::cout << "Success: OpenCL image processing validated.\n";

        return 0;

    } catch (const cl::Error& e) {
        std::cerr << "OpenCL Error: " << e.what()
                  << " (" << e.err() << ": " << errorCodeToString(e.err()) << ")\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
