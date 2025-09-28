#include <iostream>
#include <opencv2/core/ocl.hpp>
#include "util/util.hpp"


int main() {
    std::cout << std::boolalpha << "OpenCL available: " << cv::ocl::haveOpenCL() << std::endl;
    if (cv::ocl::haveOpenCL()) {
        cv::ocl::setUseOpenCL(true);
    }
    const std::string opencl_device_descriptions = get_opencl_device_descriptions();
    std::cout << opencl_device_descriptions << std::endl;
}
