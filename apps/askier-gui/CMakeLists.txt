SET(SOURCE_LIST
        main.cpp
)

add_executable(askier-gui ${SOURCE_LIST})

target_compile_features(askier-gui PUBLIC cxx_std_23)
target_compile_options(askier-gui PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(askier-gui PUBLIC ../../include)

target_link_libraries(askier-gui PUBLIC gui)
# Add the build include dir so the generated header can be found
target_include_directories(askier-gui PUBLIC ${PROJECT_BINARY_DIR}/include)