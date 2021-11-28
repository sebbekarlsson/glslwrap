cmake_minimum_required(VERSION 3.20)

add_compile_options(-g -Wall)

project(glslwrap)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/.)



add_custom_target(
  glslwrap_shaders ALL
  COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/.
)




file(GLOB PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB glslwrap_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)


add_executable(glslwrap_e ${glslwrap_sources})

# just for debugging purposes
target_link_libraries(glslwrap_e PRIVATE glfw)



list(FILTER glslwrap_sources EXCLUDE REGEX "main.c")

add_library(glslwrap SHARED ${glslwrap_sources})
add_library(glslwrap_static STATIC ${glslwrap_sources})

target_compile_options(glslwrap PUBLIC -fPIC)
target_compile_options(glslwrap_static PUBLIC -fPIC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(
  glslwrap
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_include_directories(
  glslwrap_static
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_include_directories(
  glslwrap_e
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


include(FetchContent)

FetchContent_Declare(
  cglm
  GIT_REPOSITORY https://github.com/recp/cglm.git
  GIT_TAG "v0.8.4"
  )
if (NOT TARGET cglm)
  FetchContent_MakeAvailable(cglm)
endif()

FetchContent_Declare(
  hashmap_static
  GIT_REPOSITORY https://github.com/sebbekarlsson/hashmap.git
  GIT_TAG master
  )

if (NOT TARGET hashmap_static)
  FetchContent_MakeAvailable(hashmap_static)
endif()

target_link_libraries(glslwrap_static PUBLIC m GL GLEW cglm hashmap_static)
target_link_libraries(glslwrap_e PUBLIC m GL GLEW cglm hashmap_static)
target_link_libraries(glslwrap PUBLIC m GL GLEW cglm hashmap_static)
