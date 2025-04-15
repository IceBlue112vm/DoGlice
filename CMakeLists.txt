cmake_minimum_required(VERSION 3.10)
include(_.cmake)

project(DGI)

ae2f_CoreLibFetch(ae2f Core main)

file(GLOB_RECURSE src_DGI ${PROJECT_SOURCE_DIR}/src/DGI/*.c*)
file(GLOB_RECURSE include_DGI ${PROJECT_SOURCE_DIR}/include/DGI/*)


add_library(DGI_Interface INTERFACE ${include_DGI})
target_link_libraries(Core INTERFACE ${include_DGI})

add_executable(DGI ${src_DGI})
target_link_libraries(DGI PRIVATE DGI_Interface)
target_link_libraries(DGI PRIVATE vulkan)
