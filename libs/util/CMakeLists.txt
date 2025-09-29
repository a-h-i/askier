file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/util/*.hpp")

SET(SOURCE_LIST
        OpenCLDeviceDescription.cpp
)

add_library(util ${SOURCE_LIST} ${HEADER_FILES})

target_compile_features(util PUBLIC cxx_std_23)
target_compile_options(util PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>: /O2 /DNDEBUG>
        $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-O3 -DNDEBUG -march=native>

)
target_include_directories(util PUBLIC ../../include)


find_package(OpenCL REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core )
target_link_libraries(util PUBLIC OpenCL::OpenCL Qt6::Core)

# Add the build include dir so the generated header can be found
target_include_directories(util PUBLIC ${PROJECT_BINARY_DIR}/include)