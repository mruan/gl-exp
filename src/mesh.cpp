#include "mesh.hpp"
#include "math_util.hpp"

Mesh::Mesh(std::map<std::string, unsigned int>& skel_def)
  :mBone2TfIdx(skel_def), mBoneOffset(skel_def.size())
{
  _VAO = 0;
  memset(_Buffers, 0, sizeof(_Buffers));
  //  _numBone = 0;
  //  _pSceneMesh = NULL;
}

Mesh::~Mesh()
{
  Clear();
}

const std::vector<glm::mat4>& Mesh::GetBoneOffset()
{
  return mBoneOffset;
}

void Mesh::Clear()
{
  /*
    for (unsigned int i = 0 ; i < _textures.size() ; i++) 
    {
    if(_textures[i])
    {
    delete _texture[i];
    _textures[i] = 0;
    }
    }
  */
  if (_Buffers[0] != 0)
    glDeleteBuffers(sizeof(_Buffers)/sizeof(_Buffers[0]), _Buffers);
  
  if (_VAO != 0)
    {
      glDeleteVertexArrays(1, &_VAO);
      _VAO = 0;
    }
}

bool Mesh::InitBonesFromMesh(const aiMesh* pMesh, 
			     std::vector<VtxBoneInfo>& bones)
{
  // Fill OffsetMatrix
  for(uint i=0; i< pMesh->mNumBones; i++)
    {
      uint boneIdx =0;
      std::string BoneNm(pMesh->mBones[i]->mName.data);
      
      if(mBone2TfIdx.find(BoneNm) != mBone2TfIdx.end())
	{
	  boneIdx = mBone2TfIdx[BoneNm];
	  CopyMat(pMesh->mBones[i]->mOffsetMatrix, mBoneOffset[boneIdx]);
	  
	  //printf("Idx: %2u, Name: %s\n", boneIdx, BoneNm.c_str());
	  //pprintMat16(mBoneOffset[boneIdx]);
	  }
      else
	{
	  //printf("Not needed: %s\n", BoneNm.c_str());
	}

      // Fill weights:
      for (unsigned int j=0; j< pMesh->mBones[i]->mNumWeights; j++)
	{
	  unsigned int Vid = pMesh->mBones[i]->mWeights[j].mVertexId;
	  float weight = pMesh->mBones[i]->mWeights[j].mWeight;
	  //  printf("vid = %u, weight = %f\n", Vid, weight);
	  bones[Vid].AddBoneData(boneIdx, weight);
	}
    }
  return true;
}

void Mesh::RenderMesh()
{
  glBindVertexArray(_VAO);
  
  //      glDrawElements(GL_TRIANGLES, _entries[i].numIdx, GL_UNSIGNED_INT, 0);
  
  glDrawElementsBaseVertex(GL_TRIANGLES, mNumIdx,
			   GL_UNSIGNED_INT, (void*) 0, 0);

  glBindVertexArray(0); 
}

bool Mesh::InitMesh(const aiMesh* pMesh)
{
  // Do a clean load
  Clear();

  // Create the VAO
  glGenVertexArrays(1, &_VAO);
  glBindVertexArray(_VAO);

  // Create the buffers for the vertices:
  glGenBuffers(sizeof(_Buffers)/sizeof(_Buffers[0]), _Buffers);

  // TODO: inverse the global transform
  //_globalInvTf = glm::affineInverse();

  mNumIdx = pMesh->mNumFaces*3;

  // Temporary storage for some info:
  std::vector<float> pos, nrm, tex;
  std::vector<VtxBoneInfo> bones;
  std::vector<unsigned int> idx;
  pos.reserve(3*pMesh->mNumVertices);
  nrm.reserve(3*pMesh->mNumVertices);
  tex.reserve(2*pMesh->mNumVertices);
  bones.resize(pMesh->mNumVertices);
  idx.reserve(3*pMesh->mNumFaces);

  const aiVector3D zero3D(0.0f, 0.0f, 0.0f);
  for(unsigned int i=0; i< pMesh->mNumVertices; i++)
    {
      const aiVector3D* pPos = &(pMesh->mVertices[i]);
      const aiVector3D* pNor = &(pMesh->mNormals[i]);
      const aiVector3D* pTex = pMesh->HasTextureCoords(0)?
			&(pMesh->mTextureCoords[0][i]): &zero3D;
      
      pos.push_back(pPos->x); pos.push_back(pPos->y); pos.push_back(pPos->z);
      nrm.push_back(pNor->x); nrm.push_back(pNor->y); nrm.push_back(pNor->z);
      tex.push_back(pTex->x); tex.push_back(pTex->y);
    }

  if (pMesh->HasBones())
    InitBonesFromMesh(pMesh, bones);
  else
    printf("Mesh does not contain bones\n");
  
  for(unsigned int i=0; i< pMesh->mNumFaces; i++)
    {
      aiFace& f = pMesh->mFaces[i];
      assert(f.mNumIndices ==3);
      idx.push_back(f.mIndices[0]);
      idx.push_back(f.mIndices[1]);
      idx.push_back(f.mIndices[2]);
    }

  printf("Fill in gl arrays\n");
  // Generate and populate the buffers with vertex attributes and indices
  glBindBuffer(GL_ARRAY_BUFFER, _Buffers[POS_VB]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*pos.size(), &pos[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  
  glBindBuffer(GL_ARRAY_BUFFER, _Buffers[TEXCOORD_VB]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tex.size(), &tex[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
  
  glBindBuffer(GL_ARRAY_BUFFER, _Buffers[NORMAL_VB]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*nrm.size(), &nrm[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
  
  glBindBuffer(GL_ARRAY_BUFFER, _Buffers[BONE_VB]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(bones[0])*bones.size(), &bones[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(3); // ID
  glVertexAttribIPointer(3, NUM_BONES_PER_VERTEX, GL_INT, sizeof(VtxBoneInfo), 0);
  glEnableVertexAttribArray(4); // Weights
  //  const int offset = sizeof(float)*NUM_BONES_PER_VERTEX;
  glVertexAttribPointer(4, NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VtxBoneInfo), (const void*) 16);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _Buffers[INDEX_BUFFER]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*idx.size(), &idx[0], GL_STATIC_DRAW);

  // In the end, make sure the VAO is not changed from outside
  glBindVertexArray(0);
  return (glGetError() == GL_NO_ERROR);
}

/************************************************************************
 ************************************************************************
               Other structs and classes
 ************************************************************************
 ***********************************************************************/
/*
void BoneTf::BoneTf()
{
  Offset  = glm::mat4(0.0f);
  FinalTf = glm::mat4(1.0f);
}
*/
