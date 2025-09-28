SET(SOURCE_LIST
        main.cpp
)

add_executable(askier-gui ${SOURCE_LIST})

target_compile_features(askier-gui PUBLIC cxx_std_23)
target_compile_options(askier-gui PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>: /O2 /DNDEBUG>
        $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-O3 -DNDEBUG -march=native>
)
target_include_directories(askier-gui PUBLIC ../../include)

target_link_libraries(askier-gui PUBLIC gui)
# Add the build include dir so the generated header can be found
target_include_directories(askier-gui PUBLIC ${PROJECT_BINARY_DIR}/include)