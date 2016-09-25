// SnowRacer
// Kristoffer 2009-04

// tyre.cpp

#include "tyre.h"
#include <cmath>
#define PI 3.1416
#define PHISTEPS 30
#define THSTEPS 10
#define TEXPERLAP 4.0

Tyre::Tyre(GLfloat rad)
{
  // beräkna och spara geometri
  radius = rad;
  vertices   = new GLfloat[PHISTEPS*THSTEPS + 2][3];
  normals    = new GLfloat[PHISTEPS*THSTEPS + 2][3];
  texCoords  = new GLfloat[PHISTEPS*THSTEPS + 2][2];
  indexes    = new unsigned int*[THSTEPS +1];
  indexes[0] = new unsigned int[PHISTEPS +1];
  indexes[1] = new unsigned int[PHISTEPS +1];         // triangle-fans
  for (int i = 2; i < THSTEPS +1; ++i)
    indexes[i] = new unsigned int[PHISTEPS*2];        // triangle-strips
  
  GLfloat phi, theta, s, t;
  GLfloat d = rad/2;
  
  for (int thstep = 0; thstep < THSTEPS; ++thstep)
  {
    theta = PI/(THSTEPS-1) * thstep;
    s = .25 + .5/(THSTEPS-1) * thstep;
    
    for (int phistep = 0; phistep < PHISTEPS; ++phistep)
    {
      phi = 2*PI/(PHISTEPS-1) * phistep;
      t = TEXPERLAP/(PHISTEPS-1) * phistep;  
      
      vertices[2 + thstep*PHISTEPS + phistep][1] = d*sin(theta)*sin(phi)+d*sin(phi);
      vertices[2 + thstep*PHISTEPS + phistep][2] = d*sin(theta)*cos(phi)+d*cos(phi);
      vertices[2 + thstep*PHISTEPS + phistep][0] = d*cos(theta);
         
      normals[2 + thstep*PHISTEPS + phistep][1] = sin(theta)*sin(phi);
      normals[2 + thstep*PHISTEPS + phistep][2] = sin(theta)*cos(phi);
      normals[2 + thstep*PHISTEPS + phistep][0] = cos(theta);
      
      texCoords[2 + thstep*PHISTEPS + phistep][0] = s;
      texCoords[2 + thstep*PHISTEPS + phistep][1] = t;
    }
  }
  
  vertices[0][1] = 0;
  vertices[0][2] = 0;
  vertices[0][0] = d;
  
  normals[0][1] = 0;
  normals[0][2] = 0;
  normals[0][0] = 1;
  
  texCoords[0][0] = .5;
  texCoords[0][1] = .0;
  
  vertices[1][1] = 0;
  vertices[1][2] = 0;
  vertices[1][0] =-d;

  normals[1][1] = 0;
  normals[1][2] = 0;
  normals[1][0] =-1;
  
  texCoords[0][0] = .5;
  texCoords[0][1] = 1.0;
  
  
  // triangle-fan 1
  indexes[0][0] = 0;
  for (int i = 0; i < PHISTEPS; ++i)
    indexes[0][i+1] = 2 + PHISTEPS-i-1;
  
  // triangle-fan 2
  indexes[1][0] = 1;
  for (int i = 0; i < PHISTEPS; ++i)
    indexes[1][i+1] = 2 + (THSTEPS-1)*PHISTEPS + i;  
    
  // triangle-strips
  for (int strip = 0; strip < THSTEPS-1; ++strip)
  {
    for (int i = 0; i < PHISTEPS; ++i)
    {
      indexes[2 + strip][2*i+1] = 2 +  strip   *PHISTEPS + i;
      indexes[2 + strip][2*i  ] = 2 + (strip+1)*PHISTEPS + i;
    }
  }         
}


Tyre::~Tyre()
{
  delete[] vertices;
  delete[] normals;
  delete[] texCoords;
  
  for (int i = 0; i < THSTEPS +1; ++i)
    delete[] indexes[i];
    
  delete[] indexes;
}


void Tyre::Draw()
{
  glPushMatrix();
  glTranslatef(position[0], position[1]+radius, position[2]);
  glRotatef(-wSteerAngle/PI*180.0, .0, 1.0, .0);
  glRotatef(wRotation/PI*180.0, -1.0, .0, .0);
  
  // texturen redan laddad av bilen
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
     
  glVertexPointer(3, GL_FLOAT, 0, vertices);  
  glNormalPointer(GL_FLOAT, 0, normals);
  glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
  
  glDrawElements(GL_TRIANGLE_FAN, PHISTEPS+1, GL_UNSIGNED_INT, indexes[0]);
  glDrawElements(GL_TRIANGLE_FAN, PHISTEPS+1, GL_UNSIGNED_INT, indexes[1]);
  
  for (int strip = 0; strip < THSTEPS-1; ++strip)
    glDrawElements(GL_TRIANGLE_STRIP, PHISTEPS*2, GL_UNSIGNED_INT, indexes[2 + strip]);
  
  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glPopMatrix();
}
