#include "util/util.hpp"
#include <sstream>
#include <vector>
#include <CL/opencl.hpp>

std::string get_opencl_device_descriptions() {
    std::ostringstream stream;
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    stream << std::boolalpha;

    for (const auto &platform: platforms) {
        stream << "Platform Name: " << platform.getInfo<CL_PLATFORM_NAME>() << "\n";
        stream << "Platform Vendor: " << platform.getInfo<CL_PLATFORM_VENDOR>() << "\n";
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);
        for (const auto &device: devices) {
            stream << "\tDevice: " << device.getInfo<CL_DEVICE_NAME>() << "\n";
            stream << "\t\tDevice Vendor: " << device.getInfo<CL_DEVICE_VENDOR>() << "\n";
            stream << "\t\tDevice Version: " << device.getInfo<CL_DEVICE_VERSION>() << "\n";
            stream << "\t\tDevice Max Compute Units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << "\n";
            stream << "\t\tDevice Global Memory: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() << "\n";
            stream << "\t\tDevice Max Clock Frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() <<
                    "\n";
            stream << "\t\tDevice Max Memory Allocation: " << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() <<
                    "\n";
            stream << "\t\tDevice Local Memory: " << device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() << "\n";
            stream << "\t\tDevice Available: " << device.getInfo<CL_DEVICE_AVAILABLE>() << "\n";

            switch (device.getInfo<CL_DEVICE_TYPE>()) {
                case CL_DEVICE_TYPE_GPU:
                    stream << "\t\tDevice Type: GPU" << "\n";
                    break;
                case CL_DEVICE_TYPE_CPU:
                    stream << "\t\tDevice Type: CPU" << "\n";
                    break;
                default:
                    stream << "\t\tDevice Type: unknown" << "\n";
            }
        }
    }
    return stream.str();
}
