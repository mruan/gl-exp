/*
This program loads one Collada file for mesh and animation.
It is meant to test the basic functionalities of this pipeline.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep
#include <map>

// ASSIMP
#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>

#include "animation.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "window.hpp"

#define OBJECT_SOURCE_FILE  "/home/ming/Work/GL/Workspace/gl-exp/resource/run.dae"
#define SHADER_SOURCE_FILE  "/home/ming/Work/GL/Workspace/gl-exp/resource/shader.glsl"
#define TEXTURE_SOURCE_FILE "./"

#define WIDTH  800
#define HEIGHT 600

glm::vec3 initPos(5.0f, 0.0f, 0.0f);
glm::vec3 initTar(0.0f, 0.0f, 0.0f);
glm::vec3 initUp(0.0f, 0.0f, 1.0);

std::map<std::string, unsigned int> skel_ref;
// Make a reference skeleton;
void MakeSkel(std::map<std::string, unsigned int>& mBoneIdx);

int main(int argc, char** argv)
{
  // Init OpenGL and window related stuff
  Window window("main", WIDTH, HEIGHT);
  // Init the camera
  Camera camera(60.f, 4/3.f, 0.5f, 10.0f, initPos, initUp, initTar);
  // Init the shader
  Shader shader;
  if (!shader.InitShader(SHADER_SOURCE_FILE))
    return -1;
  //  if (!shader.InitTextureTGA(TEXTURE_SOURCE_FILE))
  //    return -1;

  // Build a reference skeleton:
  MakeSkel(skel_ref);

  // Init the ref mesh skeleton, not ready to load the actual object yet
  Mesh   myMesh(skel_ref);
  // Init the ref anim skeleton, not ready to load the actual animation yet
  Animation anim(skel_ref, myMesh.GetBoneOffset());

  // ASSIMP importer, load the scene which consists the mesh and animation
  Assimp::Importer importer;
  const aiScene* pScene = importer.ReadFile(OBJECT_SOURCE_FILE, 0u);
  if (pScene == NULL)
    {
      printf("Scene failed to be loaded, exit\n");
      return -1;
    }
  // Init the mesh
  if(pScene->HasMeshes())
    myMesh.InitMesh(pScene->mMeshes[0]);
  
  // Init the animation
  if(pScene->HasAnimations())
    anim.InitAnimation(pScene->mRootNode, pScene->mAnimations[0]);

  int frame = 0;
  do{
    window.ClearCanvas();
    shader.Enable();

    // DO SOMETHING
#define INTERVAL 1.0 // 1fps
    if(window.ShouldShowNextFrame(INTERVAL))
      {
	anim.UpdateFrameTfs(frame++);
	shader.SetBoneTfs(anim.GetTfs());
      }

    camera.updateViewFromInput();
    glm::mat4 worldModel = glm::mat4(1.0f);//assume model matrix is I
    shader.SetWVP(camera.getMVP(worldModel));

    myMesh.RenderMesh();

    window.SwapBuffer();
  }while(window.IsRunning());

  return 0;
}

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

/*
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
*/
