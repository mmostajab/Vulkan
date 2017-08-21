PROJECT(VkMeshLoader-core)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0016 NEW)

MESSAGE(STATUS "Configuring VkMeshLoader Source")

# VULKAN
FIND_PACKAGE(glfw3  REQUIRED)
FIND_PACKAGE(GLM    REQUIRED)
FIND_PACKAGE(VULKAN REQUIRED)

# sources from core directories
set(VkMeshLoaderSources
	Application.cpp
	plydatareader.cpp
	rply.cpp
	main.cpp
    helper.cpp
    VkRenderer.cpp
)

set(VkMeshLoaderHeaders
	Application.h
	plydatareader.h
	rply.h
	AABB.h
	helper.h
    VkRenderer.h
)

include_directories(${Vulkan_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIR})

ADD_DEFINITIONS(${VkMeshLoaderGlobalDefinitions})

INCLUDE_DIRECTORIES(${VkMeshLoaderIncludeDirs})

add_executable(VkMeshLoader
	${VkMeshLoaderSources}
	${VkMeshLoaderHeaders}
)

find_package(Eigen3 REQUIRED)
TARGET_LINK_LIBRARIES(VkMeshLoader ${Vulkan_LIBRARY} glfw)
target_include_directories(VkMeshLoader PRIVATE ${EIGEN3_INCLUDE_DIR})

add_custom_target(copy-VkMeshLoader-files ALL
    COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/bin/data
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/glsl ${CMAKE_BINARY_DIR}/bin/glsl
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/shaders ${CMAKE_BINARY_DIR}/bin/shaders
    DEPENDS ${MY_TARGET}

	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/glsl ${CMAKE_BINARY_DIR}/glsl
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/shaders ${CMAKE_BINARY_DIR}/shaders
    DEPENDS ${MY_TARGET}
	)
add_dependencies(VkMeshLoader copy-VkMeshLoader-files)

message("${CMAKE_SOURCE_DIR}")
message("${CMAKE_BINARY_DIR}")

DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VkMeshLoaderSources ${VkMeshLoaderHome} "")
DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VkMeshLoaderHeaders ${VkMeshLoaderHome} "")
