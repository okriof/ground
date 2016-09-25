// SnowRacer
// Kristoffer 2009-04

// sysdep.h
// funktioner som använder operativsystemspecifika funktioner
// sysdep.cpp finns i två olika varianter

#ifndef SYSDEP_H
#define SYSDEP_H

#include <GL/glut.h>

class Sysdep
{
  public:
    static Sysdep* Instance();
    
    double getElapsedTime();
    
    void   loadShader();
    
    void   setGroundDrawMode();  // aktiverar shadern i linux-varianten
    void   setOthersDrawMode();
    
          ~Sysdep();
  private:
    Sysdep();
    Sysdep(const Sysdep&);
    Sysdep& operator= (const Sysdep&);

    static Sysdep* pinstance;
    double oldTime;
    
    GLuint shaderProgram;
  };

#endif
