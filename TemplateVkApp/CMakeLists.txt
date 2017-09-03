#
# StreamLineGenerator - Yet another visualization framework
# CMake file inspired by Voreen
#

PROJECT(TemplateVkApp)
CMAKE_MINIMUM_REQUIRED(VERSION 3.3 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/externals/glm;${PROJECT_SOURCE_DIR}/externals/glfw")

set(VkTemplateHome "${PROJECT_SOURCE_DIR}")

#
# Further Build Configurations
#

MESSAGE(STATUS "--------------------------------------------------------------------------------")

ADD_SUBDIRECTORY(src)
