#include <stdio.h>
#include <string.h>
#include <string>
#include <assert.h>
#include <glfx.h>

#include "shader.hpp"

#include <GL/glfw.h>

using namespace std;

Shader::Shader()
{
  m_shaderProg = 0;
  m_effect = glfxGenEffect();
}

Shader::~Shader()
{
  if (m_shaderProg != 0)
    {
      glDeleteProgram(m_shaderProg);
      m_shaderProg = 0;
    }
    
  glfxDeleteEffect(m_effect); 
}

bool Shader::CompileProgram(const char* pEffectFile, const char* pProgram)
{
    if (!glfxParseEffectFromFile(m_effect, pEffectFile)) {
        string log = glfxGetEffectLog(m_effect);
        printf("Error creating effect from file '%s':\n", pEffectFile);
        printf("%s\n", log.c_str());
        return false;
    }
    
    m_shaderProg = glfxCompileProgram(m_effect, pProgram);
    
    if (m_shaderProg < 0) {
        string log = glfxGetEffectLog(m_effect);
        printf("Error compiling program '%s' in effect file '%s':\n", pProgram, pEffectFile);
        printf("%s\n", log.c_str());
        return false;
    }
    
    return true;
}

void Shader::Enable()
{
    glUseProgram(m_shaderProg);
}


GLint Shader::GetUniformLocation(const char* pUniformName)
{
    GLuint Location = glGetUniformLocation(m_shaderProg, pUniformName);

    if (Location == INVALID_OGL_VALUE) {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }

    return Location;
}

GLint Shader::GetProgramParam(GLint param)
{
    GLint ret;
    glGetProgramiv(m_shaderProg, param, &ret);
    return ret;
}

/*************************************************************************************************
Technique class
*************************************************************************************************/
bool Shader::InitShader(const char* pEffectFile)
{
  // Compile the effect glsl file
  if(!CompileProgram(pEffectFile, "Shading"))
    return false;

  // Set up memory locations for view and texture
  m_WVPLocation = GetUniformLocation("gWVP");
  m_ColorTextureLocation = GetUniformLocation("gColorMap");

  // Set up memory blocks for bone transforms
  for(unsigned int i=0; i< MAX_BONES; i++)
    {
      char name[32];
      memset(name, 0, sizeof(name));
      sprintf(name, "gBones[%d]", i);
      m_boneLocation[i] = GetUniformLocation(name);
    }

  return true;
}

bool Shader::InitTextureTGA(const char* pTextureTGA)
{
  // Create one OpenGL texture
  GLuint textureID;
  glGenTextures(1, &textureID);
  
  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, textureID);
  
  // Read the file, call glTexImage2D with the right parameters
  int flag = glfwLoadTexture2D(pTextureTGA, 0);
  if (flag != GL_TRUE)
    {
      printf("Error parsing texture input file %s\n", pTextureTGA);
      return false;
    }

  // Nice trilinear filtering.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
  glGenerateMipmap(GL_TEXTURE_2D);
  
  // TODO: for now set the texture here
  SetColorTextureUnit(textureID);
  // Return the ID of the texture we just created
  return true;//textureID;
}

void Shader::SetWVP(const glm::mat4& wvp)
{
  glUniformMatrix4fv(m_WVPLocation, 1, GL_FALSE, (const GLfloat*) &wvp[0][0]);
}

void Shader::SetColorTextureUnit(GLuint Texture)
{
  // Bind our texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, Texture);
  // Set our "gcolormap" sampler to user Texture Unit 0
  glUniform1i(m_ColorTextureLocation, 0);
}

void Shader::SetBoneTfs(const std::vector<glm::mat4>& Tfs)
{
  assert(Tfs.size() < MAX_BONES);
  for(unsigned int i=0; i < Tfs.size(); i++)
    glUniformMatrix4fv(m_boneLocation[i], 1, GL_FALSE, 
		       (const GLfloat*) &Tfs[i][0][0]);
}
