// SnowRacer
// Kristoffer 2009-04

// camera.cpp


#include "camera.h"
#include "ground.h"
#include <cmath>
#include "demodefines.h"

#define CAMERAMODES 3
#define FOLLOWSPEED .05
#define FOLLOWDIST 14
#define PI 3.1416

Camera::Camera()
{    
  //nollställ
  for(unsigned char r = 0; r < 16; ++r)
    cameraMatrix[r] = 0;
  cameraMatrix[15] = 1;
  
  for(unsigned char r = 0; r < 3; ++r)
    position[r] = 0;
  position[3] = 1;
  
  cameraMode = 0;
  viewphi = 0;
  viewth = 0;
  goalphi = 0;
  goalth = 0;
}


void Camera::changeMode()  // byt kameratyp
{
  if (++cameraMode >= CAMERAMODES)
    cameraMode = 0;
}


void Camera::viewDirection(GLfloat phi, GLfloat th)  // önskad tittriktning
{
  goalphi = -phi;
  goalth = -th;
}


void Camera::setCamera()
{     
  glLoadMatrixf(cameraMatrix);  // ladda view-matris
  
  // uppdatera culling-plan
  for( int i = 0; i < 4; ++i)
    Camera2World3f(localFrustumNormals[i], &frustumPlanesNormals[3*i]); 
    
  Camera2World4f(localFrustumPoint, frustumPlanesPoint);
  for( int i = 0; i < 4; ++i)
    frustumPointNormalProd[i] = frustumPlanesNormals[3*i  ]*frustumPlanesPoint[0] +
                                frustumPlanesNormals[3*i+1]*frustumPlanesPoint[1] +
                                frustumPlanesNormals[3*i+2]*frustumPlanesPoint[2];
                                  
  // test-uppsättning för att visa culling
  #ifdef VISIBLE_FRUSTUM_CULLING  
  glLoadIdentity();
  glTranslatef(0, 0, -15);
  glMultMatrixf(cameraMatrix); 
  #endif
}
 
 
void Camera::setProjection(GLfloat le, GLfloat bo, GLfloat ne, GLfloat fa)
{
  glMatrixMode(GL_PROJECTION);
  
  for(unsigned char r = 0; r < 16; ++r)
    projectionMatrix[r] = 0;
  
  projectionMatrix[0] = -ne/le;
  projectionMatrix[5] = -ne/bo;      
  projectionMatrix[10] = -(fa+ne)/(fa-ne);
  projectionMatrix[11] = -1;
  projectionMatrix[14] = -2*fa*ne/(fa-ne);
  
  glLoadMatrixf(projectionMatrix);
  
  glMatrixMode(GL_MODELVIEW);
  
  // frustum-culling-plan i kamerakoordinater
  localFrustumPoint[0] = 0;
  localFrustumPoint[1] = 0;
  localFrustumPoint[2] = 0;
  localFrustumPoint[3] = 1;
  
  localFrustumNormals[0][0] =-ne;//left
  localFrustumNormals[0][1] = 0;
  localFrustumNormals[0][2] =-le;
  
  localFrustumNormals[1][0] = ne;//right
  localFrustumNormals[1][1] = 0;
  localFrustumNormals[1][2] =-le;
  
  localFrustumNormals[2][0] = 0;//bottom
  localFrustumNormals[2][1] =-ne;
  localFrustumNormals[2][2] =-bo;
  
  localFrustumNormals[3][0] = 0;//top
  localFrustumNormals[3][1] = ne;
  localFrustumNormals[3][2] =-bo;
  
  localFrustumNormals[4][0] = 0;//(far)
  localFrustumNormals[4][1] = 0;
  localFrustumNormals[4][2] =-1;
  
  // normera
  for (int i = 0; i < 4; ++i)
  {
    GLfloat len = sqrt( localFrustumNormals[i][0]*localFrustumNormals[i][0] +
                        localFrustumNormals[i][1]*localFrustumNormals[i][1] +
                        localFrustumNormals[i][2]*localFrustumNormals[i][2] );
    localFrustumNormals[i][0] /= len;
    localFrustumNormals[i][1] /= len;
    localFrustumNormals[i][2] /= len;
  }
  
}
 
 
void  Camera::loadIdentity()
{    
  for(unsigned char r = 0; r < 15; r++)
    cameraMatrix[r] = 0;
  cameraMatrix[0] = 1;
  cameraMatrix[5] = 1;
  cameraMatrix[10] = 1;
  cameraMatrix[15] = 1;
}



void  Camera::translate(GLfloat x, GLfloat y, GLfloat z)
{
  GLfloat t[16];     
  // temporär matris
  for(unsigned char r = 0; r < 16; r++)
    t[r] = cameraMatrix[r];

  cameraMatrix[12] = t[0]*x + t[4]*y + t[8]*z + t[12];
  cameraMatrix[13] = t[1]*x + t[5]*y + t[9]*z + t[13];
  cameraMatrix[14] = t[2]*x + t[6]*y + t[10]*z + t[14];
  cameraMatrix[15] = t[3]*x + t[7]*y + t[11]*z + t[15];
}



void  Camera::multMatrix(GLfloat m[])
{
  GLfloat t[16];     
  
  for(unsigned char r = 0; r < 16; r++)
    t[r] = cameraMatrix[r];

  cameraMatrix[0] = t[0]*m[0] + t[4]*m[1] + t[8]*m[2] + t[12]*m[3];
  cameraMatrix[1] = t[1]*m[0] + t[5]*m[1] + t[9]*m[2] + t[13]*m[3];
  cameraMatrix[2] = t[2]*m[0] + t[6]*m[1] + t[10]*m[2] + t[14]*m[3];
  cameraMatrix[3] = t[3]*m[0] + t[7]*m[1] + t[11]*m[2] + t[15]*m[3];

  cameraMatrix[4] = t[0]*m[4] + t[4]*m[5] + t[8]*m[6] + t[12]*m[7];
  cameraMatrix[5] = t[1]*m[4] + t[5]*m[5] + t[9]*m[6] + t[13]*m[7];
  cameraMatrix[6] = t[2]*m[4] + t[6]*m[5] + t[10]*m[6] + t[14]*m[7];
  cameraMatrix[7] = t[3]*m[4] + t[7]*m[5] + t[11]*m[6] + t[15]*m[7];

  cameraMatrix[8] = t[0]*m[8] + t[4]*m[9] + t[8]*m[10] + t[12]*m[11];
  cameraMatrix[9] = t[1]*m[8] + t[5]*m[9] + t[9]*m[10] + t[13]*m[11];
  cameraMatrix[10] = t[2]*m[8] + t[6]*m[9] + t[10]*m[10] + t[14]*m[11];
  cameraMatrix[11] = t[3]*m[8] + t[7]*m[9] + t[11]*m[10] + t[15]*m[11];

  cameraMatrix[12] = t[0]*m[12] + t[4]*m[13] + t[8]*m[14] + t[12]*m[15];
  cameraMatrix[13] = t[1]*m[12] + t[5]*m[13] + t[9]*m[14] + t[13]*m[15];
  cameraMatrix[14] = t[2]*m[12] + t[6]*m[13] + t[10]*m[14] + t[14]*m[15];
  cameraMatrix[15] = t[3]*m[12] + t[7]*m[13] + t[11]*m[14] + t[15]*m[15];
}


 
void  Camera::informCarPosition(GLfloat position_N[], GLfloat phi, GLfloat th, GLfloat v) 
// bilen berättar var den är
{
  loadIdentity();
  viewphi = viewphi*(1-FOLLOWSPEED) + goalphi*FOLLOWSPEED;
  viewth  = viewth *(1-FOLLOWSPEED) + goalth *FOLLOWSPEED;

  GLfloat grHeight;
  GLfloat len;
  
  GLfloat t[16];
  for(unsigned char r = 0; r < 15; r++)
    t[r] = 0;
  t[15] = 1;
  
  switch (cameraMode)
  {
  case 0: // följande
  case 1: // följande hög
    cameraMatrix[5] =  cos(viewth);
    cameraMatrix[6] =  -sin(viewth);
    
    cameraMatrix[9 ] =  sin(viewth);
    cameraMatrix[10] =  cos(viewth);
    position[0] = position[0] * (1-FOLLOWSPEED) + (position_N[0] + FOLLOWDIST*sin(phi+viewphi)) * FOLLOWSPEED;
    position[2] = position[2] * (1-FOLLOWSPEED) + (position_N[2] + FOLLOWDIST*cos(phi+viewphi)) * FOLLOWSPEED;
    
    if (cameraMode == 1)
      position[1] = position[1] * (1-FOLLOWSPEED) + (position_N[1] + FOLLOWDIST*2) * FOLLOWSPEED;
    else
      position[1] = position[1] * (1-FOLLOWSPEED) + (position_N[1] + FOLLOWDIST/3) * FOLLOWSPEED;
    
    
    grHeight = theGround->Height(position);
    if (position[1]-3 < grHeight)
      position[1] = grHeight +3;
      
    
    // bygg matris (riktning från bil)
    t[2 ] = position[0] - position_N[0];
    t[6 ] = position[1] - position_N[1];
    t[10] = position[2] - position_N[2];
    len = sqrt ( t[2 ]*t[2 ] +
                 t[6 ]*t[6 ] +
                 t[10]*t[10] );
    t[2 ] /= len;
    t[6 ] /= len;
    t[10] /= len;
      
    // höger
    t[0] = t[10];
    t[4] = 0;
    t[8] =-t[2 ];
    len = sqrt ( t[0]*t[0] +
                 t[8]*t[8] );
    t[0] /= len;
    t[8] /= len;
    
    // uppåt
    t[1] = t[6 ]*t[8];
    t[5] = t[10]*t[0] - t[2]*t[8];
    t[9] =            - t[6]*t[0];
    
    multMatrix(t);  
    translate(-position[0], -position[1], -position[2]);
    break;
    
  case 2: 
    // åkande
    cameraMatrix[0] =  cos(viewphi);
    cameraMatrix[1] =  sin(viewphi)*sin(viewth);
    cameraMatrix[2] =  sin(viewphi)*cos(viewth);
     
    cameraMatrix[5] =  cos(viewth);
    cameraMatrix[6] =  -sin(viewth);
    
    cameraMatrix[8 ] = -sin(viewphi);
    cameraMatrix[9 ] =  cos(viewphi)*sin(viewth);
    cameraMatrix[10] =  cos(viewphi)*cos(viewth);
        
    t[0] =  cos(phi)*cos(v) + sin(phi)*sin(th)*sin(v);
    t[1] = -cos(phi)*sin(v) + sin(phi)*sin(th)*cos(v);
    t[2] =  sin(phi)*cos(th);
    
    t[4] =  cos(th)*sin(v);
    t[5] =  cos(th)*cos(v);
    t[6] =  -sin(th);
    
    t[8 ] = -sin(phi)*cos(v) + cos(phi)*sin(th)*sin(v);
    t[9 ] =  sin(phi)*sin(v) + cos(phi)*sin(th)*cos(v);
    t[10] =  cos(phi)*cos(th);
    
    translate(0,-1.1, 2.0 );  // kameraposition i bil
    multMatrix(t);
    translate(-position_N[0], -position_N[1], -position_N[2]);
    
    Camera2World4f(localFrustumPoint, position);
    break;
  }
}
 
 
void Camera::Camera2World4f(GLfloat input[], GLfloat output[])
{ // förutsätter cameraMatrix endast rotation + translation
    output[0] = (input[0]-cameraMatrix[12]) * cameraMatrix[0] +
                (input[1]-cameraMatrix[13]) * cameraMatrix[1] +
                (input[2]-cameraMatrix[14]) * cameraMatrix[2];
    output[1] = (input[0]-cameraMatrix[12]) * cameraMatrix[4] +
                (input[1]-cameraMatrix[13]) * cameraMatrix[5] +
                (input[2]-cameraMatrix[14]) * cameraMatrix[6];
    output[2] = (input[0]-cameraMatrix[12]) * cameraMatrix[8] +
                (input[1]-cameraMatrix[13]) * cameraMatrix[9] +
                (input[2]-cameraMatrix[14]) * cameraMatrix[10];
    output[3] = 1;
}
 
 
void Camera::Camera2World3f(GLfloat input[], GLfloat output[])
{
    output[0] = input[0] * cameraMatrix[0] +
                input[1] * cameraMatrix[1] +
                input[2] * cameraMatrix[2];
    output[1] = input[0] * cameraMatrix[4] +
                input[1] * cameraMatrix[5] +
                input[2] * cameraMatrix[6];
    output[2] = input[0] * cameraMatrix[8] +
                input[1] * cameraMatrix[9] +
                input[2] * cameraMatrix[10];
}
