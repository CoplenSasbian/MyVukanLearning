
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB_RECURSE window_srcs CONFIGURE_DEPENDS "src/*.cpp")


set(LIBNAME vkd)

add_library(${LIBNAME} SHARED
	${window_srcs}
)


target_include_directories(${LIBNAME} PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/include
)


find_package(proxy CONFIG REQUIRED)
find_package(mimalloc CONFIG REQUIRED)
find_package(stdexec  CONFIG REQUIRED)
target_link_libraries(${LIBNAME} PUBLIC msft_proxy mimalloc STDEXEC::stdexec)


include(GenerateExportHeader)
generate_export_header(${LIBNAME}
BASE_NAME  ${LIBNAME}
EXPORT_MACRO_NAME  DLL_API
EXPORT_FILE_NAME 
${CMAKE_CURRENT_LIST_DIR}/include/${LIBNAME}/window_export.h )