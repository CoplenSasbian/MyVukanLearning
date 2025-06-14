cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 23)

project(vulkan_demo )


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(libs)

file(GLOB_RECURSE  APP_SRC "src/*.cpp" "src/*.h")

add_executable(vulkan_demo
	${APP_SRC}
)
find_package(mimalloc REQUIRED)
target_link_libraries(vulkan_demo PRIVATE platform mimalloc)


