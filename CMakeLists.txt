cmake_minimum_required(VERSION 3.8)
project(starforge)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

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

find_package(Iconv REQUIRED)
find_package(CURL REQUIRED)

add_executable(starforge ${STARFORGE_SRC})
target_include_directories(starforge PUBLIC include include/util lib/glfw/include lib/ImGuiFileDialog/ImGuiFileDialog/ lib/libgctools/include lib/fmt/include ${Iconv_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})

if(UNIX)
target_link_libraries(starforge PUBLIC j3dultra imgui glfw gctools fmt ${ICONV_LIBRARIES} ${CURL_LIBRARIES})
endif(UNIX)

if(WIN32)
target_link_libraries(starforge PUBLIC j3dultra imgui glfw gctools fmt ${ICONV_LIBRARIES} ${CURL_LIBRARIES} ssp)
endif(WIN32)
