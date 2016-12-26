PROJECT(SimpleVkApplication-core)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0016 NEW)

MESSAGE(STATUS "Configuring SimpleVkApplication Source")

# VULKAN
FIND_PACKAGE(glfw3  REQUIRED)
FIND_PACKAGE(GLM    REQUIRED)
FIND_PACKAGE(VULKAN REQUIRED)

# sources from core directories
set(SimpleVkApplicationSources
	Application.cpp
	plydatareader.cpp
	rply.cpp
	main.cpp
	arcball.cpp
	zoom.cpp
	Navigation.cpp
	translationPan.cpp
       
        VkRenderer.cpp
)

set(SimpleVkApplicationHeaders
	Application.h
	plydatareader.h
	rply.h
	AABB.h
	helper.h
	arcball.h
	zoom.h
	Navigation.h
	translationPan.h

        VkRenderer.h
)

include_directories(${Vulkan_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIR})

ADD_DEFINITIONS(${SimpleVkApplicationGlobalDefinitions})

INCLUDE_DIRECTORIES(${SimpleVkApplicationIncludeDirs})

add_executable(SimpleVkApplication
	${SimpleVkApplicationSources}
	${SimpleVkApplicationHeaders}
)

find_package(Eigen3 REQUIRED)
TARGET_LINK_LIBRARIES(SimpleVkApplication ${Vulkan_LIBRARY} glfw)
target_include_directories(SimpleVkApplication PRIVATE ${EIGEN3_INCLUDE_DIR})

add_custom_target(copy-SimpleVkApplication-files ALL
    COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/bin/data
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/glsl ${CMAKE_BINARY_DIR}/bin/glsl
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    DEPENDS ${MY_TARGET}
	COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/glsl ${CMAKE_BINARY_DIR}/glsl
    DEPENDS ${MY_TARGET}
	)
add_dependencies(SimpleVkApplication copy-SimpleVkApplication-files)

message("${CMAKE_SOURCE_DIR}")
message("${CMAKE_BINARY_DIR}")

DEFINE_SOURCE_GROUPS_FROM_SUBDIR(SimpleVkApplicationSources ${SimpleVkApplicationHome} "")
DEFINE_SOURCE_GROUPS_FROM_SUBDIR(SimpleVkApplicationHeaders ${SimpleVkApplicationHome} "")
