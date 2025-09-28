#include <QApplication>
#include "gui/MainWindow.hpp"
#include "askier/version.hpp"
#include <string>
#include <algorithm>
#include <opencv2/core/ocl.hpp>
#include <iostream>
#include <CL/opencl.hpp>

int main(int argc, char **argv) {
    std::cout << std::boolalpha << "OpenCL available: " << cv::ocl::haveOpenCL() << std::endl;
    if (cv::ocl::haveOpenCL()) {
        cv::ocl::setUseOpenCL(true);
    }
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    for (const auto &platform: platforms) {
        std::cout << "Platform Name: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
        std::cout << "Platform Vendor: " << platform.getInfo<CL_PLATFORM_VENDOR>() << std::endl;
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);
        for (const auto &device: devices) {
            std::cout << "\tDevice: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
            std::cout << "\t\tDevice Vendor: " << device.getInfo<CL_DEVICE_VENDOR>() << std::endl;
            std::cout << "\t\tDevice Version: " << device.getInfo<CL_DEVICE_VERSION>() << std::endl;
            std::cout << "\t\tDevice Max Compute Units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
            std::cout << "\t\tDevice Global Memory: " << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() << std::endl;
            std::cout << "\t\tDevice Max Clock Frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() <<
                    std::endl;
            std::cout << "\t\tDevice Max Memory Allocation: " << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() <<
                    std::endl;
            std::cout << "\t\tDevice Local Memory: " << device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() << std::endl;
            std::cout << "\t\tDevice Available: " << device.getInfo<CL_DEVICE_AVAILABLE>() << std::endl;

            switch (device.getInfo<CL_DEVICE_TYPE>()) {
                case CL_DEVICE_TYPE_GPU:
                    std::cout << "\t\tDevice Type: GPU" << std::endl;
                    break;
                case CL_DEVICE_TYPE_CPU:
                    std::cout << "\t\tDevice Type: CPU" << std::endl;
                    break;
                default:
                    std::cout << "\t\tDevice Type: unknown" << std::endl;
            }
        }
    }
    std::cout << std::noboolalpha;

    QApplication app(argc, argv);
    const std::string appname = ASKIER_NAME;
    std::string appname_lower;
    std::transform(appname.begin(), appname.end(), appname_lower.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    app.setApplicationName(appname_lower.c_str());
    app.setApplicationVersion(ASKIER_VERSION);

    MainWindow window;
    window.show();
    return app.exec();
}
