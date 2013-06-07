/*
  Given a reference skeleton and a complete bvh file, 
  generate a reduced version of it.
 */

// Get access to all bvh functions...
#include "bvhProc.hpp"
#include "mesh.hpp"

#include <stdio.h>
#include <stdlib.h>

#define PREFIX "/home/ming/Work/data/cmuconvert/"
#define FOLDER "01/"
#define FILE   "01_01.bvh"

#define OUTPUT "/home/ming/Work/data/mocap/"
int main()
{
  std::map<std::string, unsigned int> dict = Mesh::BuildRefSkeleton();
  BvhPreProcessor bpp(1.0); // 1 sec per sample

  std::string prefix(PREFIX);
  std::string output(OUTPUT);

  //  FILE* fd;
  bpp.ParseAnimation((prefix+FOLDER+FILE).c_str());

  bpp.DumpAnimToFile( (output+FOLDER+FILE).c_str());
}
