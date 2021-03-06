PROJECT(VkTemplate-core)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0016 NEW)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR ${VkTemplateHome}/externals/VulkanMemoryAllocator/src)

# include macros
INCLUDE(${VkTemplateHome}/cmake/macros.cmake)

MESSAGE(STATUS "Configuring VkTemplate Source")

# Window system
FIND_PACKAGE(glfw3  REQUIRED)
# Math Lib
FIND_PACKAGE(GLM    REQUIRED)
# VULKAN
FIND_PACKAGE(VULKAN REQUIRED)
# Asset Importer
FIND_PACKAGE(ASSIMP REQUIRED)

# sources from core directories
set(VkTemplateSources
	Application.cpp
	Application_Graphics.cpp
	Application_Compute.cpp
	plydatareader.cpp
	rply.cpp
	main.cpp
	helper.cpp
	VkRenderer.cpp
	GraphicsPipeline.cpp
	ComputePipeline.cpp
	MeshLoader.cpp
	Buffer.cpp
	BufferAllocator.cpp
	Shader.cpp
)

set(VkTemplateHeaders
	Application.h
	plydatareader.h
	rply.h
	AABB.h
	helper.h
	VkRenderer.h
	GraphicsPipeline.h
	ComputePipeline.h
	MeshLoader.h
	Buffer.h
	BufferAllocator.h
	Shader.h
)

include_directories(${Vulkan_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} ${ASSIMP_INCLUDE_DIR} ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIR})

ADD_DEFINITIONS(${VkTemplateGlobalDefinitions})

INCLUDE_DIRECTORIES(${VkTemplateIncludeDirs})

add_executable(VkTemplate
	${VkTemplateSources}
	${VkTemplateHeaders}
)

TARGET_LINK_LIBRARIES(VkTemplate ${Vulkan_LIBRARY} glfw ${ASSIMP_LIBRARY_RELEASE})

set_target_properties(VkTemplate PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

add_custom_target(copy-VkTemplate-files ALL
    COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/bin/data
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/shaders ${CMAKE_BINARY_DIR}/bin/shaders
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/glsl ${CMAKE_BINARY_DIR}/bin/glsl
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/externals/shaderCompilers ${CMAKE_BINARY_DIR}/bin/shaderCompilers
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/externals/assimp/bin/assimp-vc140-mt.dll ${CMAKE_BINARY_DIR}/bin/Debug/
	DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/externals/assimp/bin/assimp-vc140-mt.dll ${CMAKE_BINARY_DIR}/bin/
	DEPENDS ${MY_TARGET}
)
add_dependencies(VkTemplate copy-VkTemplate-files)

DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VkTemplateSources ${VkTemplateHome} "")
DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VkTemplateHeaders ${VkTemplateHome} "")
