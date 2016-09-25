// lånad från labbserien
#ifndef helpers_h
#define helpers_h


#include <GL/gl.h>


struct Model
{
  GLfloat* vertexArray;
  GLfloat* normalArray;
  GLfloat* texCoordArray;
  GLfloat* colorArray;
  GLuint* indexArray;
  int numVertices;
  int numIndices;
};


// Loads a model from disk. OBJ format is supported.
// Exits program if there is any error.
Model* loadModel(const char* name);

#endif
