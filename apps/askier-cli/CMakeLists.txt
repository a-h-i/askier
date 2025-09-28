
set(SOURCE_LIST
    main.cpp
)

add_executable(askier-cli ${SOURCE_LIST})


target_compile_features(askier-cli PUBLIC cxx_std_23)
target_compile_options(askier-cli PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>: /O2 /DNDEBUG>
        $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-O3 -DNDEBUG -march=native>
)
target_include_directories(askier-cli PUBLIC ../../include)

target_link_libraries(askier-cli PUBLIC askier util)
# Add the build include dir so the generated header can be found
target_include_directories(askier-cli PUBLIC ${PROJECT_BINARY_DIR}/include)