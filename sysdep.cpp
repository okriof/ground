// SnowRacer
// Kristoffer 2009-04

// sysdep.cpp

#ifdef WIN32
  //windows
  #include <ctime>
#else
  // linux/unix-versionen
  #include <sys/time.h>
  #define GL_GLEXT_PROTOTYPES
  #include <GL/glut.h>
  #include "shaderutils.h"
#endif

#include "sysdep.h"
#include <iostream>

using namespace std;


Sysdep* Sysdep::pinstance = 0;

Sysdep* Sysdep::Instance() 
{
  if (pinstance == 0)
    pinstance = new Sysdep;
    
  return pinstance;
}

Sysdep::~Sysdep()
{
  delete pinstance;
  pinstance = 0;
}

#ifdef WIN32
  //windows
  Sysdep::Sysdep() 
  { 
    oldTime =  (clock()/((double)CLOCKS_PER_SEC));
  }

  double Sysdep::getElapsedTime()
  {
    double newTime = (clock()/((double)CLOCKS_PER_SEC));
    double dt = newTime - oldTime;
    oldTime = newTime;
    return dt;
  }

  void Sysdep::loadShader()
  {
    cout << "Using standard pipeline" << endl;
  }

  void Sysdep::setGroundDrawMode() {}

  void Sysdep::setOthersDrawMode() {}


#else
  // linux

Sysdep::Sysdep() 
{ 
  struct timeval timeVal;
  gettimeofday(&timeVal, 0);
  oldTime = (double) timeVal.tv_sec + (double) timeVal.tv_usec * 0.000001;
  
  shaderProgram = 0;
}


double Sysdep::getElapsedTime()
{
  struct timeval timeVal;
  gettimeofday(&timeVal, 0);
  double newTime = (double) timeVal.tv_sec + (double) timeVal.tv_usec * 0.000001;
  double dt = newTime - oldTime;
  
  oldTime = newTime;
  
  return dt;
}


void Sysdep::loadShader()
{
  // Create vertex shader
  GLuint vertexShader = createShaderFromFile(GL_VERTEX_SHADER, "ground.vs");
  if (!vertexShader)
  {  // reverting to standard pipeline
    cout << "Using standard pipeline." << endl;
    shaderProgram = 0; 
    return;
  }

  // Create fragment shader
  GLuint fragmentShader = createShaderFromFile(GL_FRAGMENT_SHADER, "ground.fs");
  if (!fragmentShader)
  {  // reverting to standard pipeline
    cout << "Using standard pipeline." << endl;
    shaderProgram = 0; 
    return;
  }

  // Create shader program
  shaderProgram = createShaderProgram(vertexShader, fragmentShader);
  if (!shaderProgram)
  {  // reverting to standard pipeline
    cout << "Using standard pipeline." << endl;
    shaderProgram = 0; 
    return;
  }
}


void Sysdep::setGroundDrawMode()
{
  glUseProgram(shaderProgram);
}

void Sysdep::setOthersDrawMode()
{
  glUseProgram(0);
}
#endif
