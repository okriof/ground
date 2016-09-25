//lånad från labbserien

#include "helpers.h"
#include "loadobj.h"
#include "string.h"
#include <iostream>


Model* loadModel(const char* name)
{
  Model* model = 0;
  int nameLen = strlen(name);

  std::cout << "Loading model " << name << "\n";

  if (nameLen >= 4 && (!strcmp(name + nameLen - 4, ".obj")
		       || !strcmp(name + nameLen - 4, ".OBJ")))
    {
      model = loadOBJModel(name);
    }
  else
    {
      std::cerr << "Unknown file extension for file " << name << std::endl;
    }

  if (!model)
    return 0;

  std::cout << "Model has vertex colors:       " << (model->colorArray ? "Yes" : "No")    << "\n";
  std::cout << "Model has vertex normals:      " << (model->normalArray ? "Yes" : "No")   << "\n";
  std::cout << "Model has texture coordinates: " << (model->texCoordArray ? "Yes" : "No") << "\n";

  return model;
}
