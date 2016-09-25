//lånad från labbserien

#include "helpers.h"
#include "loadobj.h"
#include "string.h"
#include <iostream>


Model* loadModel(const char* name)
{
  Model* model = 0;
  int nameLen = strlen(name);

  printf("Loading model %s\n", name);

  if (nameLen >= 4 && (!strcmp(name + nameLen - 4, ".obj")
		       || !strcmp(name + nameLen - 4, ".OBJ")))
    {
      model = loadOBJModel(name);
    }
  else
    {
      fprintf(stderr, "Unknown file extension for file %s\n", name);
      fflush(stderr);
    }

  if (!model)
    return 0;

  printf("Model has vertex colors: %s\n",
	 model->colorArray ? "Yes" : "No");
  printf("Model has vertex normals: %s\n", model->normalArray ? "Yes" : "No");
  printf("Model has texture coordinates: %s\n",
	 model->texCoordArray ? "Yes" : "No");

  return model;
}
