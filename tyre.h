// SnowRacer
// Kristoffer 2009-04

// tyre.h
// Däckparametrar samt kontaktpunktsparametrar (osynliga däck..)

#ifndef TYRE_H
#define TYRE_H
#include <GL/glut.h>

class Tyre {
public:
    Tyre(GLfloat radius);
   ~Tyre(); 
       
    GLfloat position[4];
    GLfloat oldWorldPos[4];
    GLfloat radius;
    
    GLfloat steerCoefficient;
    GLfloat wSteerAngle;
    GLfloat wSpeed;
    GLfloat wRotation;
    
    bool driving; // anger om hjulet är drivande
    
    void Draw();       
       
private:
    GLfloat (*vertices)[3];
    GLfloat (*normals)[3];
    GLfloat (*texCoords)[2];
    
    unsigned int** indexes;           
};

class ContactPoint {
public:
  GLfloat position[4];
  GLfloat oldWorldPos[4];
  
private:
};

#endif
