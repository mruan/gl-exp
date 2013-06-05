
#ifndef ANIMATION_HPP
#define ANIMATION_HPP
#include <vector>

// ASSIMP libraries
#include <scene.h>
typedef unsigned int uint32;

class Animation
{
public:
  Animation(const std::map<std::string, uint32>& skel_def,
	    const std::vector<glm::mat4>& boneOffsets);

  void BuildBoneAnim(aiNode* root, aiAnimation* pAnimation);

  void UpdateFrameTfs(int frameNum);

  const std::vector<glm::mat4>& GetTfs();

private:
  aiNode* pRootNode;
  aiAnimation* pAnim;

  const std::map<std::string, unsigned int>& mBoneIdx;

  const std::vector<glm::mat4>& mBoneOffsets;
  
  std::vector<glm::mat4>  mBoneFinalTf;  

  std::vector<const aiNodeAnim*> mBoneAnim;
};

#endif
