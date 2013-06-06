
#ifndef ANIMATION_HPP
#define ANIMATION_HPP
#include <vector>
#include <map>

#include <glm/glm.hpp>

// ASSIMP libraries
#include <scene.h>
typedef unsigned int uint32;

class Animation
{
public:
  Animation(std::map<std::string, uint32>& skel_def,
	    const std::vector<glm::mat4>& boneOffsets);

  void InitAnimation(aiNode* root, aiAnimation* pAnimation);

  void UpdateFrameTfs(int frameNum);
  const std::vector<glm::mat4>& GetTfs();

private:
  void ReadNodeHeirarchy(int frameIdx,
			 const aiNode* pNode,
			 const glm::mat4& parTf);

  aiNode* pRootNode;
  aiAnimation* pAnim;

  std::map<std::string, unsigned int>& mBoneIdx;

  const std::vector<glm::mat4>& mBoneOffsets;
  
  std::vector<glm::mat4>  mBoneFinalTf;  

  std::vector<const aiNodeAnim*> mBoneAnim;
};

#endif
