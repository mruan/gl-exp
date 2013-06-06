#pragma once

// Hard code a skeleton that all subsequent reading must comply
class BvhPreProccessor
{
public:
  // Hardcoded definition of a mask
  static void BuildRefMask(std::vector<bool>& mask);

  // Consturctor
  BvhPreprocessor();

  // This function assumes the file has been successfully opened
  // if the skeleton has not been read then it is skipped directly to motion
  void ParseAnimation(FILE*& fd);

  // In contrast, this one opens the file inside the function and dump anim
  void DumpAnimToFile(const char* outfile);
private:
  double mSampleRate;  // Set it to zero if you want to keep original sample rates

  std::vector<bool> mask;
  unsigned int mNumJoints;
  
  unsigned int mNumFrames;   // number of frames for a particular bvh
  unsigned int mTotalFrames; // total number of frames processes so far
};

class BvhSkeleton
{
public:
  BvhSkeleton(std::map<std::string, unsigned int>& B2TfIdx);

  // Destructor to take care of all pointers
  ~BvhSkeleton(); 

  // Main operation: load the skeleton defined in a partial .bvh file
  bool LoadFile(const char* skel_file);

  // Accessor:
  const aiNode* GetRoot(){return mRoot;}

  const std::vector<glm::mat4> GetAllOffset(){return mBoneOffset;}

private:
  // Recursively compute the offset matrix for each bone/joint
  // starts at the root node
  void BuildOffsetMatrix(const aiNode* pNode);

  aiNode* ParseNode(FILE*& fd);
  
   std::string BvhSkeleton::GetNextToken(FILE*& fd);

  void ReadEndSite(FILE*& fd);
  void ReadChannels(FILE*& fd);
  void ReadNodeOffset(FILE*& fd, aiNode* pNode);
  
  std::map<std::string, unsigned int>& Bone2TfIdx;

  aiNode* mRoot;
  std::vector<glm::mat4> mBoneOffset;
  std::vector<aiVector3D> mBoneTr;
};

class BvhAnim
{
  BvhAnim(std::vector<aiVector3D>& BoneTf);

  bool LoadAnim(const char* file);

private:
  aiAnimation* pAnim;
  std::vector<aiVector3D>& mBoneTr;
};
