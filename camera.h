// SnowRacer
// Kristoffer 2009-04

// camera.h
// Hanterar kameran

#ifndef CAMERA_H
#define CAMERA_H
#include <GL/glut.h>

class Ground;

class Camera
{
  public:
          Camera();
    void  setCamera(); // sätt modelview-matrisen
    
    void  changeMode(); // byt kameratyp
    void  viewDirection(GLfloat phi, GLfloat th); // används för att titta runt
    
    void  setProjection(GLfloat left, GLfloat bottom, GLfloat near, GLfloat far);  // jfr glFrustum 
    
    void  setGround(Ground* ground) { theGround = ground; }; // anger pekare till marken (för att kolla kamerahöjd)
    
    void  informCarPosition(GLfloat position[], GLfloat phi, GLfloat th, GLfloat v); // bilen meddelar sin position
    
    const GLfloat* getPosition() {return position; };  // kamerans position, till rendering av marken

    GLfloat* getPlaneNormals() { return frustumPlanesNormals; };  //några frustum-plan (4 st), normaler och en punkt
    GLfloat* getPlaneNormalsPointProd() { return frustumPointNormalProd; };

    void Camera2World4f(GLfloat input[], GLfloat output[]); // transformationer mellan kamera- och världskoordinater
    void Camera2World3f(GLfloat input[], GLfloat output[]);
 
 private:
    void  loadIdentity();  // ladda enhets-view-matris
    void  multMatrix(GLfloat matrix[]);  // multiplicera med view-matris
    void  translate(GLfloat x, GLfloat y, GLfloat z);  // translation
    
    char cameraMode;
    GLfloat localFrustumNormals[5][3];
    GLfloat localFrustumPoint[4];
   
    GLfloat frustumPlanesNormals[12]; // left och right, top, bottom
    GLfloat frustumPlanesPoint[4];
    GLfloat frustumPointNormalProd[4];// left och right, top, bottom

    GLfloat position[4];
    GLfloat cameraMatrix[16];  //view-matrix
    Ground* theGround;

    GLfloat projectionMatrix[16]; //projektionsmatrisen
    
    GLfloat viewphi;
    GLfloat viewth;
    GLfloat goalphi;
    GLfloat goalth;
};

#endif
