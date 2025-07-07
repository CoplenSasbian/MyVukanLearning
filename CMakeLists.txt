cmake_minimum_required(VERSION 3.30)




project(vulkan_demo LANGUAGES CXX)
include(CTest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs)


file(GLOB_RECURSE APP_SRC "src/*.cpp")

add_executable(vulkan_demo
    ${APP_SRC}
)


find_package(mimalloc CONFIG REQUIRED)
find_package(proxy CONFIG REQUIRED)
find_package(stdexec  CONFIG REQUIRED)


target_link_libraries(vulkan_demo PRIVATE mimalloc msft_proxy STDEXEC::stdexec vkd)


add_executable(testExec test/main.cpp)
target_link_libraries(testExec PRIVATE mimalloc msft_proxy STDEXEC::stdexec vkd)