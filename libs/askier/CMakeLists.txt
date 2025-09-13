file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/askier/*.hpp")

SET(SOURCE_LIST

        VideoCaptureWorker.cpp
        GlyphDensityCalibrator.cpp
        AsciiMapper.cpp
        AsciiRenderer.cpp
        AsciiPipeline.cpp
        ImageUtils.cpp
)

add_library(askier ${SOURCE_LIST} ${HEADER_FILES})

target_compile_features(askier PUBLIC cxx_std_23)
target_compile_options(askier PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(askier PUBLIC ../../include)

find_package(OpenCV REQUIRED)

target_include_directories(askier PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(askier PUBLIC ${OpenCV_LIBS})
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Widgets)

target_link_libraries(askier PUBLIC
        Qt6::Core Qt6::Concurrent Qt6::Widgets
)
# Add the build include dir so the generated header can be found
target_include_directories(askier PUBLIC ${PROJECT_BINARY_DIR}/include)

