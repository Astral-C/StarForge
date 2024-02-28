cmake_minimum_required(VERSION 3.8)
project(starforge)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(-DGLM_ENABLE_EXPERIMENTAL)

add_subdirectory(lib)

file(GLOB_RECURSE STARFORGE_SRC
    "src/*.cpp"
    "src/*.c"
    "include/*.hpp"
    "include/*.h"
    
    "lib/imgui/misc/cpp/*.cpp"

    # ImGuiFileDialog
    "lib/ImGuiFileDialog/ImGuiFileDialog/*.cpp"
)

find_package(ICU 61.0 COMPONENTS uc i18n REQUIRED)
find_package(CURL REQUIRED)

add_executable(starforge ${STARFORGE_SRC})
target_include_directories(starforge PUBLIC include include/util lib/glfw/include lib/ImGuiFileDialog/ImGuiFileDialog/ lib/libgctools/include lib/fmt/include ${Iconv_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})


target_link_libraries(starforge PUBLIC j3dultra imgui glfw gctools fmt ICU::uc ICU::i18n CURL::libcurl)
