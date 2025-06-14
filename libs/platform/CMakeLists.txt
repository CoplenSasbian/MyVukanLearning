cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 23)

project(libplatform)

find_package(mimalloc CONFIG REQUIRED)


   


include(GenerateExportHeader)
file(GLOB_RECURSE  LIB_PLATFORM_SRC "src/*.cpp" )


add_library(platform SHARED  ${LIB_PLATFORM_SRC})
generate_export_header(platform
    EXPORT_MACRO_NAME PLT_EXPORT
    EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/platform_export.h
)

target_include_directories(platform PUBLIC include )
target_link_libraries(platform PRIVATE mimalloc)
