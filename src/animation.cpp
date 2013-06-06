#include "animation.hpp"
#include "math_util.hpp"

Animation::
Animation(std::map<std::string, uint32>& skel_def,
	  const std::vector<glm::mat4>& boneOffsets)
  :mBoneIdx(skel_def),mBoneOffsets(boneOffsets),
   mBoneFinalTf(boneOffsets.size(), glm::mat4(1.0f))
{}

void Animation::
InitAnimation(aiNode* root, aiAnimation* pAnimation)
{ 
  //    DebugMsg(2, "Counting bones in animations");
  //    printf("Counting bones in animations\n");

  // In case sometimes the root is some weird thing...
  pRootNode = root;
  std::string nodeName (pRootNode->mName.data);
  while (mBoneIdx.find(nodeName) == mBoneIdx.end())
    {
      pRootNode = pRootNode->mChildren[0];
      nodeName = std::string(pRootNode->mName.data);
    }

  // Then find the anim channels;
  pAnim = pAnimation;
  mBoneAnim.resize(mBoneIdx.size());
  for(uint32 i=0; i< pAnim->mNumChannels; ++i)
    {
      const aiNodeAnim* pNodeAnim = pAnim->mChannels[i];
      std::string name(pNodeAnim->mNodeName.data);

      // TODO: assume always finds a match
      uint32 boneIdx = mBoneIdx[name];
      mBoneAnim[boneIdx] = pNodeAnim;
      
      //	printf("Idx: %2u, Name: %s\n", boneIdx, name.c_str());
    }
}

const std::vector<glm::mat4>& Animation::GetTfs()
{
  return mBoneFinalTf;
}

void Animation::UpdateFrameTfs(int frameNum)
{
    /*
    printf("There are %d Rot %d Loc %d Scal Keys\n", 
	   pAnim->mChannels[0]->mNumRotationKeys,
	   pAnim->mChannels[0]->mNumPositionKeys,
	   pAnim->mChannels[0]->mNumScalingKeys);
    */
  
  // Recursively update the transform;
  ReadNodeHeirarchy(frameNum, pRootNode, glm::mat4(1.0f));
}

void Animation::
ReadNodeHeirarchy(int frameIdx, 
		  const aiNode* pNode, 
		  const glm::mat4& parTf)
{
  std::string nodeName(pNode->mName.data);
  if (mBoneIdx.find(nodeName) == mBoneIdx.end())
    return;
  
  uint32 boneIdx = mBoneIdx[nodeName];
  
  const aiNodeAnim* pAnim = mBoneAnim[boneIdx];
  aiVectorKey& sc= pAnim->mScalingKeys[frameIdx % pAnim->mNumScalingKeys];
  aiMatrix4x4 matScale;
  aiMatrix4x4::Scaling(sc.mValue, matScale);
  
  aiQuatKey& qt = pAnim->mRotationKeys[frameIdx % pAnim->mNumRotationKeys];
  aiMatrix4x4 matRotat(qt.mValue.GetMatrix());
  
  aiVectorKey& tr = pAnim->mPositionKeys[frameIdx % pAnim->mNumPositionKeys];
  aiMatrix4x4 matTrans;
  aiMatrix4x4::Translation(tr.mValue, matTrans);

  // Convert from aiMatrix4x4 to glm::mat4
  glm::mat4 nodeTf;
  CopyMat(matTrans * matRotat * matScale, nodeTf);

  glm::mat4 globalTf= parTf * nodeTf;
  
  glm::mat4 finalTf = globalTf * mBoneOffsets[boneIdx];
  mBoneFinalTf[boneIdx] = finalTf;
  
  // Print out info
  //    cout << nodeName << endl;
  //    pprintMat4x4(finalTf);
  // pprintScQtTr(sc.mValue, qt.mValue, tr.mValue);
  //    pprintMat16(nodeTf);
  
  for(uint i=0; i< pNode->mNumChildren; ++i)
    ReadNodeHeirarchy(frameIdx, pNode->mChildren[i], globalTf);
}

