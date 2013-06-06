/*
The goal is to loads one Collada file for mesh but tries to animate
with a bvh source
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
#define DRIVER_SOURCE_FILE  "/home/ming/Work/GL/Workspace/gl-exp/resource/walk_textured.bvh"

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
  Mesh myMesh(skel_ref);

  // ASSIMP importer, load the scene which consists the mesh and animation
  Assimp::Importer meshIp;
  const aiScene* pMeshScene = meshIp.ReadFile(OBJECT_SOURCE_FILE, 0u);
  if (pMeshScene == NULL)
    {
      printf("Scene failed to be loaded, exit\n");
      return -1;
    }
  // Init the mesh
  if(pMeshScene->HasMeshes())
    myMesh.InitMesh(pMeshScene->mMeshes[0]);
  
  // Init the ref anim skeleton, not ready to load the actual animation yet
  Assimp::Importer animIp;
  const aiScene* pAnimScene = animIp.ReadFile(DRIVER_SOURCE_FILE, 0u);

  std::vector<glm::mat4> offset = Mesh::GetOffsetFromMesh(pAnimScene->mMeshes[0], skel_ref);
  Animation anim(skel_ref, offset);
  // Init the animation
  if(pAnimScene->HasAnimations())
    anim.InitAnimation(pAnimScene->mRootNode, pAnimScene->mAnimations[0]);

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
