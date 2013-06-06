cmake_minimum_required(VERSION 2.6)
project(gl-exp)

set(CMAKE_CXX_FLAGS "-g -Wall")

# Set where the output should go
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)
#find_package(OpenCV REQUIRED)
#find_package(PCL 1.3 REQUIRED COMPONENTS common io)

add_subdirectory(external)

include_directories(
# ${PCL_INCLUDE_DIRS}
  /usr/local/include/assimp/
  /usr/local/include/glfx/
  external/glfw-2.7.6/include/
  external/glew-1.9.0/include/
  external/glm-0.9.4.0/
  .
)

link_directories(
# ${PCL_LIBRARY_DIRS}
 /usr/local/lib/
# ${CMAKE_SOURCE_DIR}/external/
)

add_definitions(
  -DTW_STATIC -DTW_NO_LIB_PRAGMA -DTW_NO_DIRECT3D -DGLEW_STATIC -D_CRT_SECURE_NO_WARNINGS)

set(GL_LIBS
  ${GLUT_LIBRARY}
  glfx
  GLFW_276
  GLEW_190
)

#add executables
add_subdirectory(src)
