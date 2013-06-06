#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include <GL/glew.h>

#include <glm/glm.hpp>

#define MAX_BONES 40

#ifndef INVALID_OGL_VALUE
#define INVALID_OGL_VALUE 0xffffffff
#endif

class Shader
{
public:
  Shader();

  ~Shader();

  bool InitShader(const char* pEffectFile);

  bool InitTextureTGA(const char* pTextureTGA);

  void Enable();

  void SetWVP(const glm::mat4& wvp);

  void SetColorTextureUnit(GLuint Texture);

  void SetBoneTfs(const std::vector<glm::mat4>& Tfs);
protected:
  bool CompileProgram(const char* pEffectFile, const char* pProgram);

  GLint GetUniformLocation(const char* pUniformName);

  GLint GetProgramParam(GLint param);

private:
  GLint m_effect;
  GLint m_shaderProg;

  const char* m_pEffectFile;

  GLuint m_WVPLocation;
  GLuint m_ColorTextureLocation;
  GLuint m_boneLocation[MAX_BONES];
};

#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF

#endif
