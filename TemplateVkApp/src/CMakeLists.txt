PROJECT(VkTemplate-core)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0016 NEW)

# include macros
INCLUDE(${VkTemplateHome}/cmake/macros.cmake)

MESSAGE(STATUS "Configuring VkTemplate Source")

# VULKAN
FIND_PACKAGE(glfw3  REQUIRED)
FIND_PACKAGE(GLM    REQUIRED)
FIND_PACKAGE(VULKAN REQUIRED)

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
)

include_directories(${Vulkan_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIR})

ADD_DEFINITIONS(${VkTemplateGlobalDefinitions})

INCLUDE_DIRECTORIES(${VkTemplateIncludeDirs})

add_executable(VkTemplate
	${VkTemplateSources}
	${VkTemplateHeaders}
)

TARGET_LINK_LIBRARIES(VkTemplate ${Vulkan_LIBRARY} glfw)

add_custom_target(copy-VkTemplate-files ALL
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
add_dependencies(VkTemplate copy-VkTemplate-files)

DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VkTemplateSources ${VkTemplateHome} "")
DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VkTemplateHeaders ${VkTemplateHome} "")
