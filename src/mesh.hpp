/*
  Process Mesh related properties from loaded Assimp Mesh
 */
#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

class Mesh
{
  //
  // Some Useful structures
  //
  struct BoneTf
  {
    glm::mat4 Offset;
    glm::mat4 FinalTf;
  };

#define NUM_BONES_PER_VERTEX 4
  typedef unsigned int uint;
  struct VtxBoneInfo
  {
    uint  ID[NUM_BONES_PER_VERTEX];
    float  W[NUM_BONES_PER_VERTEX];
  };

  //
  // Buffer layout definitions
  //
  enum VB_TYPES {
    INDEX_BUFFER,
    POS_VB,
    NORMAL_VB,
    TEXCOORD_VB,
    BONE_VB,
    NUM_VBs      
  };

public:
  // Initialize to a know definition
  Mesh(std::map<std::string, unsigned int>& skel_def);

  ~Mesh();

  void Clear();

  bool InitMesh(const aiMesh* pMesh);

  void RenderMesh();
private:
  bool InitBonesFromMesh(const aiMesh* pMesh, std::vector<VtxBoneInfo>& bones);

  GLuint _VAO;
  GLuint _Buffers[NUM_VBs];

  std::map<std::string, unsigned int>& mBone2TfIdx;

  std::vector<glm::mat4> mBoneOffset;
};

#endif
