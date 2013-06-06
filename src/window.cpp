#include "window.hpp"

#include <GL/glew.h>
#include <GL/glfw.h>
#include "camera.hpp"

Window::Window(const char* win_title, 
	       const unsigned int win_width,
	       const unsigned int win_height)
{
  // Initialise GLFW
  if( !glfwInit() )
    {
      fprintf( stderr, "Failed to initialize GLFW\n" );
      //      return 0;
    }

  glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
  glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  if( !glfwOpenWindow(win_width, win_height, 0,0,0,0, 32,0, GLFW_WINDOW ) )
    {
      fprintf( stderr, "Failed to open GLFW window\n");
      fprintf( stderr, "GPU is not 3.3 compatible. Try previous version of GL and GLSL \n" );
      glfwTerminate();
      //      return 0;
    }
  
  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    //    return 0;
  }
  
  glfwSetWindowTitle(win_title);
  
  // Ensure we can capture the escape key being pressed below
  glfwEnable( GLFW_STICKY_KEYS );
  glfwSetMousePos(win_width/2, win_height/2);
  
  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  // Cull triangles which normal is not towards the camera
  glEnable(GL_CULL_FACE);

  //  return 1;
}

Window::~Window()
{
  glfwTerminate();
}

void Window::ClearCanvas()
{
  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::SwapBuffer()
{
  glfwSwapBuffers();
}

bool Window::IsRunning()
{
  return (glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS &&
	  glfwGetWindowParam(GLFW_OPENED));
}

bool Window::ShouldShowNextFrame(double interval)
{
  static double last_time = glfwGetTime();
  double this_time = glfwGetTime();
  bool flag = false;
  if ( (this_time - last_time) > interval)
    flag = true;

  last_time = this_time;
  return flag;
}
