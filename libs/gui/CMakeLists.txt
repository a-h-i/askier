file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/gui/*.hpp")

SET(SOURCE_LIST
        MainWindow.cpp
        ConversionParamsDialog.cpp
        DoubleSlider.cpp
)

add_library(gui ${SOURCE_LIST} ${HEADER_FILES})

target_compile_features(gui PUBLIC cxx_std_23)
target_compile_options(gui PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>: /02 /DNDEBUG>
        $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-O3 -DNDEBUG -march=native>

)
target_include_directories(gui PUBLIC ../../include)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)

target_link_libraries(gui PUBLIC
        Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Concurrent
)

find_package(OpenCV REQUIRED)

target_include_directories(gui PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(gui PUBLIC ${OpenCV_LIBS})
target_link_libraries(gui PUBLIC askier)

# Add the build include dir so the generated header can be found
target_include_directories(gui PUBLIC ${PROJECT_BINARY_DIR}/include)