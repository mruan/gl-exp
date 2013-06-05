
#ifndef GL_MANAGER_HPP
#define GL_MANAGER_HPP

#include <GL/glew.h>
#include <GL/glfw.h>
#include "camera.hpp"

class Window
{
public:
  Window();

  ~Window();

  int Spin();
  
private:
  int InitWindow(const char* name, 
		 const unsigned int width, 
		 const unsigned int height);
};

#endif
