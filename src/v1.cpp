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

#define OBJECT_SOURCE_FILE  "/home/ming/Work/GL/Workspace/dae/walk_textured.dae"
#define SHADER_SOURCE_FILE  "/home/ming/Work/GL/Workspace/gl-exp/resource/shader.glsl"
#define TEXTURE_SOURCE_FILE "/home/ming/Work/GL/Workspace/texture/crp1.tga"

#define WIDTH  640
#define HEIGHT 480

glm::vec3 initPos(5.0f, 0.0f, 0.0f);
glm::vec3 initTar(0.0f, 0.0f, 0.0f);
glm::vec3 initUp(0.0f, 0.0f, 1.0);

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
  if (!shader.InitTextureTGA(TEXTURE_SOURCE_FILE))
    return -1;

  // Build a reference skeleton:
  std::map<std::string, unsigned int> skel_ref;
  skel_ref = Mesh::BuildRefSkeleton();

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

    // Animating the scene
#define INTERVAL 0.1 // 1fps
    if(window.ShouldShowNextFrame(INTERVAL))
      {
	anim.UpdateFrameTfs(frame++);
      }

    shader.SetBoneTfs(anim.GetTfs());

    camera.updateViewFromInput();
    glm::mat4 worldModel = glm::mat4(1.0f);//assume model matrix is I
    shader.SetWVP(camera.getMVP(worldModel));

    myMesh.RenderMesh();

    window.SwapBuffer();
  }while(window.IsRunning());

  return 0;
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
