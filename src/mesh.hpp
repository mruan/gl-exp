/*
  Process Mesh related properties from loaded Assimp Mesh
 */
#ifndef MESH_HPP
#define MESH_HPP

#include <cstring>
#include <string>
#include <vector>
#include <map>

#include <assimp/scene.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Mesh
{
public:
  static std::map<std::string, unsigned int> BuildRefSkeleton();
  static std::vector<glm::mat4> GetOffsetFromMesh(const aiMesh* pMesh, 
						  std::map<std::string, uint> Bone2TfIdx);

private:

#define NUM_BONES_PER_VERTEX 4
  typedef unsigned int uint;
  struct VtxBoneInfo
  {
    uint  ID[NUM_BONES_PER_VERTEX];
    float  W[NUM_BONES_PER_VERTEX];

    void Reset(){memset(ID, 0, sizeof(ID)); memset(W,  0, sizeof(W));} 

    void AddBoneData(unsigned int BoneID, float weight)
    {
      for(unsigned int i=0; i< NUM_BONES_PER_VERTEX; i++)
	{
	  if (W[i] == 0.0f)
	    {
	      ID[i] = BoneID;
	      W[i] = weight;
	      return;
	    }
	}
      
      //      printf("Error, more than 4 weights per vertex\n");
      weight /= NUM_BONES_PER_VERTEX;
      for(unsigned int i=0; i< NUM_BONES_PER_VERTEX; i++)
	{
	  W[i] += weight;
	}
      return;
      // should never arrive here (More bones than we have space for    
      assert(0);
    }
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

  const std::vector<glm::mat4>& GetBoneOffset();
private:
  bool InitBonesFromMesh(const aiMesh* pMesh, std::vector<VtxBoneInfo>& bones);

  GLuint _VAO;
  GLuint _Buffers[NUM_VBs];

  unsigned int mNumIdx;

  std::map<std::string, unsigned int>& mBone2TfIdx;

  std::vector<glm::mat4> mBoneOffset;
};

#endif
