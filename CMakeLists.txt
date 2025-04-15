cmake_minimum_required(VERSION 3.10)
include(_.cmake)

project(Doglice)
set(CMAKE_CXX_STANDARD 17)

ae2f_CoreLibFetch(ae2f Core main)
ae2f_CoreLibFetch(glfw glfw master)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE ${PROJECT_NAME}_src ${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}/*)
file(GLOB_RECURSE ${PROJECT_NAME}_inc ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*)

add_library(i${PROJECT_NAME} INTERFACE ${include_DGI})
target_link_libraries(Core INTERFACE ${${PROJECT_NAME}_inc})

target_include_directories(i${PROJECT_NAME} 
	INTERFACE include/${PROJECT_NAME} 
	INTERFACE ${Vulkan_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_src})
target_link_libraries(${PROJECT_NAME} PRIVATE i${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
