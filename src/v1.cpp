#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep

// ASSIMP
#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>


#include "shader.hpp"
#include "mesh.hpp"

#define OBJECT_SOURCE_FILE
#define SHADER_SOURCE_FILE 
#define TEXTURE_SOURCE_FILE

#WIDTH  800
#HEIGHT 600

glm::vec3 initPos(10.0f, 0.0f, 0.0f);
glm::vec3 initTar(0.0f, 0.0f, 0.0f);
glm::vec3 initUp(0.0f, 0.0f, 1.0);

std::map<std::string, unsigned int> skel_ref;

void MakeSkel(std::map<std::string, unsigned int>& mBoneIdx)
{
  mBoneIdx["Hips"]      = 0;
  mBoneIdx["LHipJoint"] = 1;
  mBoneIdx["LeftUpLeg"] = 2;
  mBoneIdx["LeftLeg"]   = 3;
  mBoneIdx["LeftFoot"]  = 4;
  mBoneIdx["RHipJoint"] = 5;
  mBoneIdx["RightUpLeg"]= 6;
  mBoneIdx["RightLeg"]  = 7;
  mBoneIdx["RightFoot"] = 8;
  mBoneIdx["LowerBack"] = 9;
  mBoneIdx["Spine"]     =10;
  mBoneIdx["Spine1"]    =11;
  mBoneIdx["Neck"]      =12;
  mBoneIdx["Neck1"]     =13;
  mBoneIdx["Head"]      =14;
  mBoneIdx["LeftShoulder"]=15;
  mBoneIdx["LeftArm"]   =16;
  mBoneIdx["LeftForeArm"]=17;
  mBoneIdx["LeftHand"]  =18;
  mBoneIdx["RightShoulder"]=19;
  mBoneIdx["RightArm"]  =20;
  mBoneIdx["RightForeArm"]=21;
  mBoneIdx["RightHand"] =22;
}

void LoadSkel(std::map<std::string, unsigned int>& rSkel_ref)
{
  std::ifstream fin("test.yaml");
  YAML::Parser parser(fin);
  YAML::Node doc;
  parser.GetNextDocument(doc);
  for(YAML::Iterator it=doc.begin();it!=doc.end();++it) 
    {
      std::string key;
      unsigned int value;
      it.first() >> key;
      it.second() >> value;
      rSkel_ref[key] = value;
      std::cout << "Key: " << key << ", value: " << value << std::endl;
    }
}

int main(int argc, char** argv)
{
  // Init OpenGL and window related stuff
  Window window("main", WIDTH, HEIGHT);
  // Init the camera
  Camera camera(60.f, 4/3.f, 0.5f, 10.0f, initPos, initUp, initTar);
  // Init the shader
  Shader shader(SHADER_SOURCE_FILE, TEXTURE_SOURCE_FILE);
  // Init the ref mesh skeleton, not ready to load the actual object yet
  Mesh   myMesh(skel_ref);
  // Init the ref anim skeleton, not ready to load the actual animation yet
  Animation anim(skel_ref, myMesh.GetBoneOffset());

  // ASSIMP importer, load the scene which consists the mesh and animation
  Assimp::Importer importer;
  aiScene* pScene = importer.ReadFile(OBJECT_SOURCE_FILE);
  // Init the mesh
  if(pScene->HasMesh())
    myMesh.InitMesh(pScene.mMeshes[0]);
  // Init the animation
  if(pScene->HasAnimation())
    anim.InitAnimation(pScene->mRootNode, pScene->mAnimations[0]);

  do{
    window.ClearCanvas();
    shader.Enable();

    // DO SOMETHING
    
    anim.UpdateFrameTfs(1);
    shader.SetBoneTfs(anim.GetTfs());

    camera.UpdateViewFromInput();
    shader.SetWVP(camera.getMVP());

    myMesh.RenderModel();

    window.SwapBuffer();
  }while(window.IsRunning());

  return 0;
}
