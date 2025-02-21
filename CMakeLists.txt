cmake_minimum_required(VERSION 3.8)
project(starforge)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(-DGLM_ENABLE_EXPERIMENTAL -DIMGUI_DEFINE_MATH_OPERATORS)

include_directories(lib/imgui) # required due to imgui file dialog being a bit finnicky
add_subdirectory(lib)

file(GLOB_RECURSE STARFORGE_SRC
    "src/*.cpp"
    "src/*.c"
    "include/*.hpp"
    "include/*.h"
    
    "lib/ImGuiFileDialog/ImGuiFileDialog/*.h"
    "lib/imgui/misc/cpp/*.cpp"
)

find_package(ICU COMPONENTS uc i18n REQUIRED)
find_package(CURL REQUIRED)

add_executable(starforge ${STARFORGE_SRC})
target_include_directories(starforge PUBLIC include include/utillib/ImGuiFileDialog/ImGuiFileDialog  lib/glfw/include lib/gctoolsplusplus/include lib/fmt/include)

target_link_libraries(starforge PUBLIC j3dultra imgui ImGuiFileDialog glfw gctools++ fmt ICU::uc ICU::i18n CURL::libcurl)
