
#ifndef GL_MANAGER_HPP
#define GL_MANAGER_HPP

class Window
{
public:
  Window(const char* name, 
	 const unsigned int width, 
	 const unsigned int height);

  ~Window();

  void ClearCanvas();

  void SwapBuffer();

  bool IsRunning();
  
  bool ShouldShowNextFrame(double interval);
  //private:

};

#endif
