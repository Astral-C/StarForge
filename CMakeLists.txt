cmake_minimum_required(VERSION 3.8)
project(starforge)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)

file(GLOB_RECURSE STARFORGE_SRC
    # Starbit
    "src/*.cpp"
    "src/*.c"
    "include/*.hpp"
    "include/*.h"
    
    # ImGuiFileDialog
    "lib/ImGuiFileDialog/ImGuiFileDialog/*.cpp"
)
find_package(ICU 61.0 COMPONENTS uc i18n REQUIRED)
add_executable(starforge ${STARFORGE_SRC})
target_include_directories(starforge PUBLIC include include/util lib/glfw/include lib/ImGuiFileDialog/ImGuiFileDialog/ lib/libgctools/include lib/fmt/include)
target_link_libraries(starforge PUBLIC j3dultra imgui glfw gctools ICU::uc ICU::i18n fmt)