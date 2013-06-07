
#include "bvhProc.hpp"
#include "math_util.hpp"

#include <matrix3x3.h>
#include <matrix4x4.h>

#define BUFFER_SIZE 512

/*********************************************************************
 *********************************************************************
                        BvhPreProcessor Class
 *********************************************************************
 ********************************************************************/
int BvhPreProcessor::BuildRefMask(std::vector<bool>& mask)
{
  // WARNING: This mask is only for the MotionBuilder version of the CMU Mocap dataset
  // The detailed definition is given below. Both the order and the mask value matter!

  // What is masked as FALSE is what defined in the bvh but not needed in our application
  // For example, fingers. They are not read into animation channels.
  /* Mask Map:
0  Hips: 1                  16 Head:   1
1  LHipJoint:   1           17 LeftShoulder:   1
2  LeftUpLeg:   1           18 LeftForeArm:    1
3  LeftLeg:     1           19 LeftArm:        1
4  LeftFoot:    1           20 LeftHand:       1
5  LeftToeBase: 0           21 LeftFingerBase: 0
6  RHipJoint:    1          22 LeftHandIndex1: 0
7  RightUpLeg:   1          23 LThumb:         0
8  RightLeg:     1          24 RightShoulder:   1
9  RightFood:    1          25 RightForeArm:    1
10 RightToeBase: 0          26 RightArm         1
11 LowerBack:    1          27 RightHand:       1
12 Spine:        1          28 RightFingerBase: 0
13 Spine1:       1          29 RightHandIndex1: 0
14 Neck:         1          30 RThumb:          0
15 Neck1:        1
  */
  
  // fill in the actual content
  // (Don't use std::copy, coz vector<bool> is different)
  mask.resize(31);
  bool m[] = {1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0};
  for(unsigned int i=0; i < 31; i++)
    mask[i] = m[i];
  // Set the actual number of valid joints:
  return 23;
}

BvhPreProcessor::BvhPreProcessor(double sampleRate)
  :mSampleRate(sampleRate), mTotalFrames(0)
{
  // Call a static function to build the reference mask
  mNumJoints = BuildRefMask(mask); // = 23
}

BvhPreProcessor::~BvhPreProcessor()
{
  Clear();
}

void BvhPreProcessor::Clear()
{
  for (unsigned int i=0; i< mFrames.size(); ++i)
    if (mFrames[i] != NULL)
      delete [] mFrames[i];
}

bool BvhPreProcessor::ParseAnimation(const char* infile)
{
  FILE* fd = fopen(infile, "r");
  if (fd == 0)
    {
      printf("Couldn't open file %s\n", infile);
      return false;
    }

  unsigned int frames;
  double frameRate;
  char buffer[BUFFER_SIZE];

  char token[32];
  
  // Find where the motion part starts
  do{
    fgets(buffer, BUFFER_SIZE, fd);
    sscanf(buffer, " %s", token); 
  }while(strcmp(token, "MOTION")!=0);
  
  fscanf(fd, " %s %u ", token, &frames); // token== "Frames:"
  
  //  fgets(buffer, BUFFER_SIZE, fd);
  fscanf(fd, " %s %s %lf ", token, token, &frameRate);

  printf("%s has %u frames at %lf\n", infile, frames, frameRate);

  // Allocate space for frames:
  mNumFrames = frames*frameRate/mSampleRate;
  mTotalFrames += mNumFrames;
  // Clear vector
  Clear();
  mFrames.resize(mNumFrames);

  int fc = 0; // frame counter;
  float x, y, z;      // Root offset
  float rx, ry, rz;   // Joint rotation
  double current_time = 0;
  double last_time=0;
  for(unsigned int i=0; i< frames; i++ )
    {
      // down sample
      if ( (current_time - last_time) > mSampleRate)
	{
    	  // Read in the root offset;
    	  fscanf(fd, " %f %f %f ", &x, &y, &z);
	  
    	  // For all the joints, read in the animation
    	  // but do not waste capacity on inactive joints
	  
	  // create a new frame:
	  //	  std::vector<float> frame;
	  try{
	    mFrames[fc] = new float[3+3*mNumJoints];
	  }catch(std::bad_alloc& ba){
	    printf("Cannot allocate more space\n");
	  }

	  float* it = &mFrames[fc][0];
	  *it = x; *(++it) = y; *(++it) = z;
    	  for(unsigned int j=0; j< mask.size(); j++)
	    {
	      // Assume the bvh file uses the Z->Y->X convention
	      fscanf(fd, " %f %f %f ", &rz, &ry, &rx);
	      
	      if (mask[j])
		{
		  // Add data to this frame:
		  *++it = rz; *++it= ry; *++it = rx; 
		}
	    }
	  // Update our sampling time
	  last_time += mSampleRate;
	  // Increment frame counter
	  ++fc;
	}
      // Finish current line and move on to the next one
      char* pc;
      do{
	fgets(buffer, BUFFER_SIZE, fd);
	pc = strrchr(buffer, '\0');
      }while(*(pc-1) !='\n');

      // Update the bvh time
      current_time += frameRate;
    }
  fclose(fd);
  return true;
}

bool BvhPreProcessor::DumpAnimToFile(const char* outfile)
{
  FILE* out = fopen(outfile, "w");
  if (out == 0)
    {
      printf("Couldn't open file %s\n", outfile);
      return false;
    }

  printf("Dump %u frames to %s\n", mNumFrames, outfile);
  fprintf(out, "Frames: %u\n",  mNumFrames);
  for(unsigned int i=0; i< mNumFrames; i++)
    {
      for(int j=0; j< 3*mNumJoints+3; j++)
	fprintf(out, "%f ", mFrames[i][j]);

      fprintf(out, "\n");
    }

  fclose(out);
  return true;
}

/*********************************************************************
 *********************************************************************
                        BvhSkeleton Class
 *********************************************************************
 ********************************************************************/
BvhSkeleton::BvhSkeleton(std::map<std::string, unsigned int>& B2TfIdx)
  :Bone2TfIdx(B2TfIdx),mBoneOffset(B2TfIdx.size()),mBoneTr(B2TfIdx.size())
{}

// A small helper function to release pointers
void BvhSkeleton::DeleteNode(aiNode* pNode)
{
  if(pNode->mNumChildren ==0)
    delete pNode;

  for(unsigned int i=0; i< pNode->mNumChildren; i++)
    DeleteNode(pNode->mChildren[i]);
}

BvhSkeleton::~BvhSkeleton()
{
  DeleteNode(mRoot);
  mRoot = 0;
}

bool BvhSkeleton::LoadFile(const char* skel_file)
{
  FILE* fd = fopen(skel_file, "r");

  // TODO: check the file is actually opened

  char buffer[BUFFER_SIZE];

  // Read in "Hierarchy"
  fscanf(fd, " %s", buffer);
  if (strcmp(buffer, "HIERARCHY") !=0)
    {
      fprintf(stderr, "BVH file must start with HIERARCHY\n");
      return false;
    }
  // finish the rest of the line
  fgets(buffer, BUFFER_SIZE, fd);
  
  std::string root = GetNextToken(fd);
  if (root != "ROOT")
    {
      printf("Expected root node\n");
      return false;
    }
  mRoot = ReadNode(fd);
  assert(mRoot!=0);

  // Compute mBoneOffset for each node
  BuildAllOffset(mRoot);
  return true;
}

void BvhSkeleton::BuildAllOffset(const aiNode* pNode)
{
  std::string name(pNode->mName.C_Str());
  if(Bone2TfIdx.find(name) != Bone2TfIdx.end())
    {
      // calculate the bone offset matrix by concatenating the inverse transformations of all parents
      aiMatrix4x4 mOffsetMatrix = aiMatrix4x4( pNode->mTransformation).Inverse();
      for( aiNode* parent = pNode->mParent; parent != NULL; parent = parent->mParent)
	{
	  mOffsetMatrix = aiMatrix4x4( parent->mTransformation).Inverse() * mOffsetMatrix;
	}

      unsigned int boneIdx = Bone2TfIdx[name];
      CopyMat(mOffsetMatrix, mBoneOffset[boneIdx]);

      for(unsigned int i=0; i< pNode->mNumChildren; i++)
	BuildAllOffset(pNode->mChildren[i]);
    }
}

aiNode* BvhSkeleton::ReadNode(FILE*& fd)
{
  // first token is name
  std::string nodeName = GetNextToken(fd);
  if (nodeName.empty() || nodeName == "{")
    {
      printf("Expect node name but got: %s\n", nodeName.c_str());
      return NULL;
    }
  
  // then an opening brace should follow
  std::string openBrace = GetNextToken(fd);
  if (openBrace != "{")
    {
      printf("Expect opening brace, but got %s\n", openBrace.c_str());
      return NULL;
    }

  // Craete a node
  aiNode* node = new aiNode( nodeName);
  std::vector<aiNode*> childNodes;

  while(1)
    {
      std::string token = GetNextToken(fd);

      // node offset to parent node
      if ( token == "OFFSET")
	ReadNodeOffset(fd, node);
      else if (token == "CHANNELS")
	ReadChannels(fd);
      else if (token == "JOINT")
	{
	  // child node follows
	  aiNode* child = ReadNode(fd);
	  child->mParent = node;
	  childNodes.push_back(child);
	}
      else if (token == "End")
	ReadEndSite(fd);
      else if (token == "}")
	break;
      else
	{
	  printf("It should never reach here!\n");
	  return NULL;
	}
    }

  // add the child nodes if there are any
  if(childNodes.size() > 0)
    {
      node->mNumChildren = childNodes.size();
      node->mChildren = new aiNode*[node->mNumChildren];
      std::copy(childNodes.begin(), childNodes.end(), node->mChildren);
    }
  return node;
}

// ------------------------------------------------------------------------------------------------
// Retrieves the next token
std::string BvhSkeleton::GetNextToken(FILE*& fd)
{
  char buffer[BUFFER_SIZE];
  int status;// = fscanf(fd, " %s", buffer);

  do{
    status = fscanf(fd, " %s", buffer);
    assert(status > 0);
  }while(buffer[0]=='\n');

  if(buffer[status-1] == '\n')
    buffer[status-1] = '\0';

  return std::string(buffer);
}

// ------------------------------------------------------------------------------------------------
// Reads an end node and returns the created node.
void BvhSkeleton::ReadEndSite(FILE*& fd)
{
  // check opening brace
  std::string openBrace = GetNextToken(fd);
  if( openBrace != "{")
    {
      printf("Expect opening brace, but got %s\n", openBrace.c_str());
      return;
    }

  // Create a node
  aiNode* node = new aiNode( "EndSite_");

  // now read the node's contents. Only possible entry is "OFFSET"
  while( 1)
    {
      std::string token = GetNextToken(fd);
      
      // end node's offset
      if( token == "OFFSET")
	{
	  ReadNodeOffset(fd, node);
	} 
      else if( token == "}")
	{
	  // we're done with the end node
	  break;
	} 
      else
	{
	  printf("It should never reach here\n");
	}
    }
  delete node;
  // and return the sub-hierarchy we built here
  return;
}

// ------------------------------------------------------------------------------------------------
// Reads a node offset for the given node
void BvhSkeleton::ReadNodeOffset(FILE*& fd,  aiNode* pNode)
{
  // Offset consists of three floats to read
  float x, y, z;
  fscanf(fd, " %f %f %f", &x, &y, &z); 
  //  offset.x = GetNextTokenAsFloat(fd);
  //  offset.y = GetNextTokenAsFloat(fd);
  //  offset.z = GetNextTokenAsFloat(fd);
  
  mBoneTr[Bone2TfIdx[pNode->mName.C_Str()]] = aiVector3D(x,y,z);

  // build a transformation matrix from it
  pNode->mTransformation = aiMatrix4x4( 1.0f, 0.0f, 0.0f, x, 0.0f, 1.0f, 0.0f, y,
					0.0f, 0.0f, 1.0f, z, 0.0f, 0.0f, 0.0f, 1.0f);
}

// ------------------------------------------------------------------------------------------------
// Reads the animation channels for the given node
void BvhSkeleton::ReadChannels(FILE*& fd)
{
  // Given the way all BVH motion data are loaded.
  int numChannels;
  fscanf(fd, " %d", &numChannels);
  
  std::string channelToken = GetNextToken(fd);
  if (channelToken =="Xposition")//this happens for root node
    {
      GetNextToken(fd);
      GetNextToken(fd);
    }
  else
    {
      assert( channelToken == "Zrotation");
      channelToken = GetNextToken(fd);
      assert( channelToken == "Yrotation");
      channelToken = GetNextToken(fd);
      assert( channelToken == "Xrotation");
    }
}

/*********************************************************************
 *********************************************************************
                          BvhAnim Class
 *********************************************************************
 ********************************************************************/
BvhAnim::BvhAnim(std::vector<aiVector3D>& BoneTr)
  :mBoneTr(BoneTr)
{
  pAnim = new aiAnimation();
  pAnim->mNumChannels = BoneTr.size();
}

BvhAnim::~BvhAnim()
{
  for(unsigned int j=0; j < pAnim->mNumChannels; j++)
    {
      delete [] pAnim->mChannels[j];
    }
  delete pAnim;
}

bool BvhAnim::LoadAnim(const char* file)
{
  FILE* fd = fopen(file, "r");
  if (fd ==0)
    {
      printf("Failed to open file %s\n", file);
      return false;
    }

  char nframe_s[32];
  int nframe;
  fscanf(fd, " %s %d", nframe_s, &nframe);

  // Pre-allocate spaces for all channels;
  for(unsigned int j=0; j < pAnim->mNumChannels; j++)
    {
      if (j== 0)// only root has translation
	pAnim->mChannels[j]->mPositionKeys = new aiVectorKey[nframe];
      else // all others just take the bone transform;
	pAnim->mChannels[j]->mPositionKeys = new aiVectorKey(0.0, mBoneTr[j]);

      pAnim->mChannels[j]->mRotationKeys = new aiQuatKey[nframe];
      pAnim->mChannels[j]->mScalingKeys  = new aiVectorKey(0.0, aiVector3D(1.0f, 1.0f, 1.0f));
    }

  for(int i=0; i < nframe; i++)
    {
      // Read in root translation
      float x,y,z;
      fscanf(fd, " %f %f %f", &x, &y, &z);
      // Don't care about the time, set all to 0.0
      pAnim->mChannels[0]->mPositionKeys[i].mValue = aiVector3D(x, y, z);
      for(unsigned int j=0; j < pAnim->mNumChannels; j++)
	{
	  aiMatrix4x4 temp;
	  aiMatrix3x3 rotMatrix;
	  //fill in the J-th joint's i-th frame:
	  // NOTE: THE ORDER MUST BE Z->Y->X!!!
	  fscanf(fd, " %f %f %f", &z, &y, &x);
	  aiMatrix4x4::RotationZ( z*PI_PER_DEG, temp); rotMatrix *= aiMatrix3x3(temp);
	  aiMatrix4x4::RotationY( y*PI_PER_DEG, temp); rotMatrix *= aiMatrix3x3(temp);
	  aiMatrix4x4::RotationX( x*PI_PER_DEG, temp); rotMatrix *= aiMatrix3x3(temp);
	  pAnim->mChannels[j]->mRotationKeys[i].mValue = aiQuaternion( rotMatrix);
	}
    }

  fclose(fd);

  return true;
}
