# Executables

set(ALL_OBJ animation.cpp camera.cpp mesh.cpp shader.cpp window.cpp)

add_executable(v1 v1.cpp ${ALL_OBJ})
target_link_libraries(v1 ${GL_LIBS} assimp)