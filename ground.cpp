// SnowRacer
// Kristoffer 2009-04

// ground.cpp

#include <iostream>

#include <cmath>
#include <cstdlib>
#include <GL/glut.h>
#include "ground.h"
#include "camera.h"
#include "demodefines.h"
#include "car.h"

#define PI 3.1416


using namespace std;

Ground::Ground(unsigned int power2initSize, GLfloat span_s, Camera* theCamerap)
{
  theCamera = theCamerap;
    
  rows = (1 << power2initSize)+1; // utgånsmarkstorlek
  cols = rows;
  span = span_s;
    
  heights  = new GLfloat[rows*cols];
  hardness = 0;
  vertices = 0;
  normals  = 0;
  levelIndexOffsets = 0;
  levelSpan = 0;
  neighbourLOD = 0;
  neighbourLOD2 = 0;
  dheightdr = 0;
  dheightdk = 0;
  theCar = 0;
  
  // nollställ höjd
  for(unsigned int r = 0; r < rows; ++r)
    for(unsigned int k = 0; k < cols; ++k)
      heights[r*cols + k] = 0; 
            
  // skapa LP- och deriveringsfilter
  lpFilt  = new GLfloat[(LPFILTRADIUS*2+1)*(LPFILTRADIUS*2+1)];
  ddrFilt = new GLfloat[(LPFILTRADIUS*2+1)*(LPFILTRADIUS*2+1)];
  ddkFilt = new GLfloat[(LPFILTRADIUS*2+1)*(LPFILTRADIUS*2+1)];
 
  //   f = 1/(sig.^2*2*pi)*exp(-(x.^2+y.^2)/(2*sig.^2));
  //   dx = -x/sig.^2 .* f;
  
  GLfloat test = 0;
  for (int r = -LPFILTRADIUS; r <= LPFILTRADIUS; ++r)
    for (int k = -LPFILTRADIUS; k <= LPFILTRADIUS; ++k)
    {
      lpFilt[(r+LPFILTRADIUS) * (LPFILTRADIUS*2+1) + k+LPFILTRADIUS] = 
          exp(-((GLfloat) (r*r + k*k))/(2*LPFILTSIGMA*LPFILTSIGMA)) /
          (LPFILTSIGMA*LPFILTSIGMA*2*PI);

      ddrFilt[(r+LPFILTRADIUS) * (LPFILTRADIUS*2+1) + k+LPFILTRADIUS] = 
          lpFilt[(r+LPFILTRADIUS) * (LPFILTRADIUS*2+1) + k+LPFILTRADIUS] *
          (-((GLfloat)r)/(LPFILTSIGMA*LPFILTSIGMA));

      ddkFilt[(r+LPFILTRADIUS) * (LPFILTRADIUS*2+1) + k+LPFILTRADIUS] = 
          lpFilt[(r+LPFILTRADIUS) * (LPFILTRADIUS*2+1) + k+LPFILTRADIUS] *
          (-((GLfloat)k)/(LPFILTSIGMA*LPFILTSIGMA));

      test += lpFilt[(r+LPFILTRADIUS) * (LPFILTRADIUS*2+1) + k+LPFILTRADIUS];
    }

  cout << "Filtsum: " << test << endl;
}



Ground::~Ground()
{
  delete[] heights;
  delete[] hardness;
  delete[] vertices;
  delete[] normals;
  delete[] levelIndexOffsets;
  delete[] levelSpan;
  delete[] neighbourLOD;
  delete[] neighbourLOD2;
  delete[] lpFilt;
  delete[] ddrFilt;
  delete[] ddkFilt;
  delete[] dheightdr;
  delete[] dheightdk;
}



void Ground::RndGroundScaleStepUp()
{
  // öka markstorleken en level
  printf("Upsampling... ");
    
  GLfloat* heightsTemp = new GLfloat[(rows*2-1)*(cols*2-1)];
    
  // nollställ
  for(unsigned int r = 0; r < (rows*2-1); r++)
    for(unsigned int k = 0; k < (cols*2-1); k++)
      heightsTemp[r*(cols*2-1) + k] = 0;
    
  // kopiera över data, 4 för att behålla höjd
  for(unsigned int r = 0; r < rows; r++)
    for(unsigned int k = 0; k < cols; k++)
      heightsTemp[r*2*(cols*2-1) + k*2] = heights[r*cols + k] *4;
    
  // ta bort gamla marknivåer
  delete[] heights;
  heights = heightsTemp;
    
  rows = rows*2-1;
  cols = cols*2-1;
    
  //filtrera
  for(int loops = 1; loops <= 7; loops++)
  {
    for(unsigned int r = 1; r < rows-1; ++r)
      for(unsigned int k = 1; k < cols-1; ++k)  
        heights[r*cols + k] = (
              heights[(r-1)*cols + (k-1)] + heights[(r-1)*cols + k] + heights[(r-1)*cols + (k+1)] +
              heights[r*    cols + (k-1)] + heights[r*    cols + k] + heights[r*    cols + (k+1)] +
              heights[(r+1)*cols + (k-1)] + heights[(r+1)*cols + k] + heights[(r+1)*cols + (k+1)])/9;
    RndGroundFixEdge();
  }
  
  printf("(%d x %d)\n", rows, cols);
}



void Ground::RndGround(GLfloat maxdev, int smoothloops)
{
  printf("Randomizing ground...\n");
  // slumpa markhöjd, ej ända ut till kanterna
  for(unsigned int r = 1; r < rows-1; ++r) 
    for(unsigned int k = 1; k < cols-1; ++k)
      heights[r*cols + k] += ((GLfloat) rand())/((GLfloat) RAND_MAX)*maxdev - maxdev/2;
           
    
  printf("Filtering...\n");        
  // filter
  for(int loops = 1; loops <= smoothloops; loops++)
  {
    for(unsigned int r = 1; r < rows-1; r++)
      for(unsigned int k = 1; k < cols-1; k++)  
        heights[r*cols + k] = (
            heights[(r-1)*cols + (k-1)] + heights[(r-1)*cols + k] + heights[(r-1)*cols + (k+1)] +
            heights[r*    cols + (k-1)] + heights[r*    cols + k] + heights[r*    cols + (k+1)] +
            heights[(r+1)*cols + (k-1)] + heights[(r+1)*cols + k] + heights[(r+1)*cols + (k+1)])/9;
    //RndGroundFixEdge();
  }
  RndGroundFixEdge();
}



void Ground::RndGroundTresh(GLfloat level)
{
  // tröskelsätt mark (allt under en viss nivå sätts till den nivån
  printf("Treshold...\n");
  for(unsigned int r = 0; r < rows; r++)
    for(unsigned int k = 0; k < cols; k++)
      if (heights[r*cols + k] < level)
          heights[r*cols + k] = level;
}   



void Ground::RndGroundFixEdge()
{
  // kopiera ut näst yttersta ramen ut till yttersta
  for(unsigned int r = 1; r < rows-1; r++)
  {  
    heights[r*cols         ] = heights[r*cols + 1];
    heights[r*cols + cols-1] = heights[r*cols + cols-2];
  }
  
  for(unsigned int k = 0; k < cols; k++)
  {  
    heights[                k] = heights[      1 *cols + k];
    heights[(rows-1)*cols + k] = heights[(rows-2)*cols + k];
  }
}
    
    
             
void Ground::RndGroundCalcVertNorm()
{
  // utgår från höjdkartan, skapar en mip-mappad version
  // beräknar punkter och ytnormaler
  unsigned int indexcount;
  unsigned int offset;
  unsigned int levelSize;
  
  //räkna detaljlevels
  levelCount = 1;
  indexcount = rows;
  while (indexcount > 3)
  {
    indexcount = (indexcount+1)/2;
    ++levelCount;
  }
  
  // hitta offsets i mipmapping-strukturen för varje level
  levelIndexOffsets = new unsigned int[levelCount+1];
  levelSpan = new GLfloat[levelCount];
  
  //indexcount = 0;
  levelIndexOffsets[0] = 0;
  for(int a = 0; a < levelCount; ++a)
  {
    levelIndexOffsets[a+1] = levelIndexOffsets[a] + ((2 << a) + 1) * ((2 << a) + 1);
  }
  
  printf("Preparing mipmap... (%d levels, %d points)\n Level:", levelCount, levelIndexOffsets[levelCount]);
  
  // allokera minne för mipmappad data
  GLfloat* mipHeights = new GLfloat[levelIndexOffsets[levelCount]];
  vertices =            new GLfloat[levelIndexOffsets[levelCount]][3];
  normals  =            new GLfloat[levelIndexOffsets[levelCount]][3];
  neighbourLOD =        new char[levelIndexOffsets[levelCount-1]];
  neighbourLOD2 =       new char[levelIndexOffsets[levelCount-1]];
  hardness =            new GLfloat[((1 << levelCount) + 1) * ((1 << levelCount) + 1)];
  dheightdr =           new GLfloat[((1 << levelCount) + 1) * ((1 << levelCount) + 1)];
  dheightdk =           new GLfloat[((1 << levelCount) + 1) * ((1 << levelCount) + 1)];
  
  for (unsigned int a = 0; a < levelIndexOffsets[levelCount-1]; ++a)
  {
    neighbourLOD[a] = 0;
    neighbourLOD2[a] = 0;
  }
  
  for (int level = levelCount-1; level >= 0; --level)
  {
    printf(" %d", level+1); cout << flush;
    offset = levelIndexOffsets[level];
    levelSize = (2 << level) + 1;
    
    if (level == levelCount-1)
    { // högsta leveln, kopiera data från heights
      levelSpan[level] = span; // varje rutas storlek
      for (unsigned int r = 0; r < levelSize; ++r)
        for (unsigned int k = 0; k < levelSize; ++k)
        {
          mipHeights[offset + r*levelSize + k] = heights[r*levelSize + k];
          hardness[r*levelSize + k] = 0;
        }
    }
    else
    { // lp-filtrera högre nivå (gaussfilter)
      levelSpan[level] = levelSpan[level+1] * 2;
      unsigned int offsetH = levelIndexOffsets[level+1];
      unsigned int levelSizeH = (2 << (level+1)) +1;
      
      convDS(&mipHeights[offsetH], levelSizeH, lpFilt, LPFILTRADIUS, &mipHeights[offset], 0, 0, levelSizeH-1, levelSizeH-1);
    }
    

    printf("v"); cout << flush;
    // vertices
    for(unsigned int r = 0; r < levelSize; ++r)
      for(unsigned int k = 0; k < levelSize; ++k)
      {
        vertices[offset + r*levelSize + k][0] = ((GLfloat) r) * levelSpan[level];
        vertices[offset + r*levelSize + k][1] = mipHeights[offset + r*levelSize + k];
        vertices[offset + r*levelSize + k][2] = ((GLfloat) k) * levelSpan[level];
      }  
       
    
    // normals
    printf("n"); cout << flush;
    GLfloat  normlen;

    conv(&mipHeights[offset], levelSize, ddrFilt, LPFILTRADIUS, dheightdr, 0, 0, levelSize-1, levelSize-1);
    conv(&mipHeights[offset], levelSize, ddkFilt, LPFILTRADIUS, dheightdk, 0, 0, levelSize-1, levelSize-1);
    
    for(unsigned int r = 0; r < levelSize; ++r)
        for(unsigned int k = 0; k < levelSize; ++k)
        {
          normals[offset + r*levelSize + k][0] = dheightdr[r*levelSize + k];
          normals[offset + r*levelSize + k][1] = levelSpan[level];
          normals[offset + r*levelSize + k][2] = dheightdk[r*levelSize + k];
        }
        
        
    for(unsigned int r = 0; r < levelSize; ++r)
        for(unsigned int k = 0; k < levelSize; ++k)        
        {
            normlen = sqrt( normals[offset + r*levelSize + k][0] * normals[offset + r*levelSize + k][0] + 
                            normals[offset + r*levelSize + k][1] * normals[offset + r*levelSize + k][1] +
                            normals[offset + r*levelSize + k][2] * normals[offset + r*levelSize + k][2]);
            
            normals[offset + r*levelSize + k][0] = normals[offset + r*levelSize + k][0]/normlen;
            normals[offset + r*levelSize + k][1] = normals[offset + r*levelSize + k][1]/normlen;
            normals[offset + r*levelSize + k][2] = normals[offset + r*levelSize + k][2]/normlen;
        }
  }
  
  // ta bort gammal höjddata
  delete[] heights;
  heights = mipHeights;
  
  printf(" done!\n"); cout << flush;
}



void Ground::Draw()
{
  // hämta kamera- och frustum-info
  camPos = theCamera->getPosition();

  planeNormals = theCamera->getPlaneNormals();
  planePointNormalProds = theCamera->getPlaneNormalsPointProd();
  
  // rita..
  // ritar och uppdaterar nästa omgångs LOD i samma rekursion
  glVertexPointer(3, GL_FLOAT, 0, vertices);
  glNormalPointer(GL_FLOAT, 0, normals);
  
  drawRec(1, 1, 1, 1);
  drawRec(1, 1, 3, 2);
  drawRec(1, 3, 1, 3);
  drawRec(1, 3, 3, 4);
  
  
  char* temp = neighbourLOD2;
  neighbourLOD2 = neighbourLOD;
  neighbourLOD = temp;
}



void Ground::drawRec(int level, unsigned int rC, unsigned int kC, unsigned char subSqNo)
{
  // kolla synlig.. (avstånd till planen) mindre än levelSpan
  unsigned int offset = levelIndexOffsets[level];
  unsigned int levelSize = (2 << level) +1;

  for (int i = 0; i < 4; ++i)
    if ( ((vertices[offset + rC*levelSize + kC][0] * planeNormals[3*i  ] +
           vertices[offset + rC*levelSize + kC][1] * planeNormals[3*i+1] +
           vertices[offset + rC*levelSize + kC][2] * planeNormals[3*i+2] ) -
           planePointNormalProds[i]) > 4*levelSpan[level])
    {
      if (level != levelCount-1)
        neighbourLOD2[offset + rC*levelSize + kC] = 0;
        
      return;
    }    

  if (level == levelCount-1)
  {
    drawFan(level, rC, kC, subSqNo);
  }
  else
  {
    if (neighbourLOD[offset + rC*levelSize + kC] == 0)
    {
      drawFan(level, rC, kC, subSqNo);
    }
    else
    {
      // rita med högre detaljnivå
      drawRec(level+1, 2*rC-1, 2*kC-1, 1);
      drawRec(level+1, 2*rC-1, 2*kC+1, 2);
      drawRec(level+1, 2*rC+1, 2*kC-1, 3);
      drawRec(level+1, 2*rC+1, 2*kC+1, 4);
    }
    
    // uppdatera neighbourLOD
    if ( ((vertices[offset + rC*levelSize + kC][0] - camPos[0]) *
          (vertices[offset + rC*levelSize + kC][0] - camPos[0]) +
          (vertices[offset + rC*levelSize + kC][1] - camPos[1]) *
          (vertices[offset + rC*levelSize + kC][1] - camPos[1]) +
          (vertices[offset + rC*levelSize + kC][2] - camPos[2]) *
          (vertices[offset + rC*levelSize + kC][2] - camPos[2])) >
          (levelSpan[level] * levelSpan[level] * MIPSWITCHCONST) )
    {
      neighbourLOD2[offset + rC*levelSize + kC] = 0; 
    }
    else
    {
      neighbourLOD2[offset + rC*levelSize + kC] = 1;
    }
  } 
}



void Ground::drawFan(int level, unsigned int rC, unsigned int kC, unsigned char subSqNo)
{
  // ritar en 2x2-ruta (9x9 punkter), fixar skarvar vid kanter mellan olika mip-nivåer
  unsigned int offset = levelIndexOffsets[level];
  unsigned int levelSize = (2 << level) +1;
  unsigned int offsetL = levelIndexOffsets[level-1];
  unsigned int levelSizeL = (2 << (level-1)) +1;
  
  
  unsigned int indices[10];
  indices[0] = offset + (rC  )*levelSize + (kC  );
  indices[1] = offset + (rC  )*levelSize + (kC+1);
  indices[2] = offset + (rC+1)*levelSize + (kC+1);
  indices[3] = offset + (rC+1)*levelSize + (kC  );
  indices[4] = offset + (rC+1)*levelSize + (kC-1);
  indices[5] = offset + (rC  )*levelSize + (kC-1);
  indices[6] = offset + (rC-1)*levelSize + (kC-1);
  indices[7] = offset + (rC-1)*levelSize + (kC  );
  indices[8] = offset + (rC-1)*levelSize + (kC+1);
  indices[9] = indices[1];
  
  
  switch (subSqNo) // ger vilka kanter som är intressanta att titta på
  {
  case 1:
    rC = (rC+1)/2;
    kC = (kC+1)/2;
    if ( (kC > 1) && (neighbourLOD[offsetL + (rC  )*levelSizeL + kC-2] == 0) )
    { // grannen till vänster har lägre upplösning
      indices[6] = offsetL + (rC-1)*levelSizeL + kC-1;
      indices[5] = indices[6];
      indices[4] = offsetL + (rC  )*levelSizeL + kC-1;
    }
    if ( (rC > 1) && (neighbourLOD[offsetL + (rC-2)*levelSizeL + kC  ] == 0) )
    { // grannen bakom har lägre upplösning
      indices[6] = offsetL + (rC-1)*levelSizeL + kC-1;
      indices[7] = indices[6];
      indices[8] = offsetL + (rC-1)*levelSizeL + kC  ;
    }
    if ( (rC > 1) && (kC > 1) && (neighbourLOD[offsetL + (rC-2)*levelSizeL + kC-2] == 0) )
    { //  grannen bakom till vänster har lägre upplösning
      indices[6] = offsetL + (rC-1)*levelSizeL + kC-1;
    }   
    break;
    
  case 2:
    rC = (rC+1)/2;
    kC = (kC-1)/2;
    if ( (kC < levelSizeL-2) && (neighbourLOD[offsetL + (rC  )*levelSizeL + kC+2] == 0) )
    { // grannen till höger har lägre upplösning
      indices[8] = offsetL + (rC-1)*levelSizeL + kC+1;
      indices[1] = indices[9] = indices[8];
      indices[2] = offsetL + (rC  )*levelSizeL + kC+1;
    }
    if ( (rC > 1) && (neighbourLOD[offsetL + (rC-2)*levelSizeL + kC  ] == 0) )
    { // grannen bakom har lägre upplösning
      indices[8] = offsetL + (rC-1)*levelSizeL + kC+1;
      indices[7] = indices[8];
      indices[6] = offsetL + (rC-1)*levelSizeL + kC  ;
    }
    if ( (rC > 1) && (kC < levelSizeL-2) && (neighbourLOD[offsetL + (rC-2)*levelSizeL + kC+2] == 0) )
    { //  grannen bakom till höger har lägre upplösning
      indices[8] = offsetL + (rC-1)*levelSizeL + kC+1;
    }  
    break;

  case 3:
    rC = (rC-1)/2;
    kC = (kC+1)/2;
    if ( (kC > 1) && (neighbourLOD[offsetL + (rC  )*levelSizeL + kC-2] == 0) )
    { // grannen till vänster har lägre upplösning
      indices[4] = offsetL + (rC+1)*levelSizeL + kC-1;
      indices[5] = indices[4];
      indices[6] = offsetL + (rC  )*levelSizeL + kC-1;
    }
    if ( (rC < levelSizeL-2) && (neighbourLOD[offsetL + (rC+2)*levelSizeL + kC  ] == 0) )
    { // grannen framför har lägre upplösning
      indices[4] = offsetL + (rC+1)*levelSizeL + kC-1;
      indices[3] = indices[4];
      indices[2] = offsetL + (rC+1)*levelSizeL + kC  ;
    }
    if ( (rC < levelSizeL-2) && (kC > 1) && (neighbourLOD[offsetL + (rC+2)*levelSizeL + kC-2] == 0) )
    { //  grannen framför till vänster har lägre upplösning
      indices[4] = offsetL + (rC+1)*levelSizeL + kC-1;
    }   
    break;

  case 4:
    rC = (rC-1)/2;
    kC = (kC-1)/2;
    if ( (kC < levelSizeL-2) && (neighbourLOD[offsetL + (rC  )*levelSizeL + kC+2] == 0) )
    { // grannen till höger har lägre upplösning
      indices[2] = offsetL + (rC+1)*levelSizeL + kC+1;
      indices[1] = indices[9] = indices[2];
      indices[8] = offsetL + (rC  )*levelSizeL + kC+1;
    }
    if ( (rC < levelSizeL-2) && (neighbourLOD[offsetL + (rC+2)*levelSizeL + kC  ] == 0) )
    { // grannen framför har lägre upplösning
      indices[2] = offsetL + (rC+1)*levelSizeL + kC+1;
      indices[3] = indices[2];
      indices[4] = offsetL + (rC+1)*levelSizeL + kC  ;
    }
    if ( (rC < levelSizeL-2) && (kC < levelSizeL-2) && (neighbourLOD[offsetL + (rC+2)*levelSizeL + kC+2] == 0) )
    { //  grannen framför till höger har lägre upplösning
      indices[2] = offsetL + (rC+1)*levelSizeL + kC+1;
    }   
    break;    
    
  default:
    break;
  }
  
  #ifndef DRAWWIRE
  glDrawElements(GL_TRIANGLE_FAN, 10, GL_UNSIGNED_INT, indices);
  #else
  glDrawElements(GL_LINE_STRIP, 10, GL_UNSIGNED_INT, indices);
  
  #ifdef DRAWNORMALS
  for (unsigned char i = 0; i < 9; ++i)
  {
    glBegin(GL_LINES);
    glVertex3f(vertices[indices[i]][0], vertices[indices[i]][1], vertices[indices[i]][2]);
    glVertex3f(vertices[indices[i]][0] + normals[indices[i]][0]/4, 
               vertices[indices[i]][1] + normals[indices[i]][1]/4, 
               vertices[indices[i]][2] + normals[indices[i]][2]/4);
    glEnd();  
  }
  #endif
  #endif
}



GLfloat Ground::Height(GLfloat position[])
{
  unsigned int offset = levelIndexOffsets[levelCount-1];
  GLfloat x = position[0]/span;
  GLfloat z = position[2]/span;
  
  // bilinear
  // kolla ej utanför..
  if (x <  0) x = 0;
  if (z <  0) z = 0;
  if (x >= rows-1) x = rows-2;
  if (z >= cols-1) z = cols-2;
    
  int nearx = (int)floor(x);
  int nearz = (int)floor(z);
    
  x = x-nearx;
  z = z-nearz;
    
  return heights[offset + (nearx  )*cols + nearz  ]*(1-x)*(1-z) +
         heights[offset + (nearx+1)*cols + nearz  ]*(  x)*(1-z) +
         heights[offset + (nearx  )*cols + nearz+1]*(1-x)*(  z) +
         heights[offset + (nearx+1)*cols + nearz+1]*(  x)*(  z);
}



void Ground::Normal(GLfloat x, GLfloat z, GLfloat* normalv)
{
  unsigned int offset = levelIndexOffsets[levelCount-1];
  x = x/span;
  z = z/span;
    
  // bilinear
  // kolla ej utanför..
  if ((x <  5) && (theCar != 0))
  {
    theCar->move(20,10,0);
    x = 5;
  }
  if ((z <  5) && (theCar != 0))
  {
    theCar->move(0,10,20);    
    z = 5;
  }
  if ((x >= rows-6) && (theCar != 0))
  {
    theCar->move(-20,10,0);    
    x = rows-7;
  }
  if ((z >= cols-6) && (theCar != 0))
  {
    theCar->move(0,10,-20);    
    z = cols-7;
  }

  // ändra x och z...

  //if (x >= rows-1) x = rows-2;
  //if (z >= cols-1) z = cols-2;
    
  int r = (int)floor(x);
  int k = (int)floor(z);

  x = x-r;
  z = z-k;
    
  GLfloat normlen = 0;
    
  for(unsigned char index = 0; index < 3; index++)
  {
    normalv[index] = normals[offset + (r  )*cols + k  ][index]*(1-x)*(1-z) +
                     normals[offset + (r+1)*cols + k  ][index]*(  x)*(1-z) +
                     normals[offset + (r  )*cols + k+1][index]*(1-x)*(  z) +
                     normals[offset + (r+1)*cols + k+1][index]*(  x)*(  z);
        normlen += normalv[index]*normalv[index];
  }
    
  normlen = sqrt(normlen);
  for(unsigned char index = 0; index < 3; index++)
    normalv[index] /= normlen;
}


void Ground::setHeight(GLfloat x, GLfloat z, GLfloat height, GLfloat radius)
{
  radius *= .25;
  unsigned int rC        = (unsigned int) round(x/span);   
  unsigned int kC        = (unsigned int) round(z/span);
  unsigned int intrad    = (unsigned int) round(radius*3/span+.501);
  unsigned int offset    = levelIndexOffsets[levelCount-1];
  unsigned int levelSize = (2 << (levelCount-1)) + 1;
  
    
  // justrera höjd i en omgivning (e^-(x^2))
  for(unsigned int r = rC-intrad; r <= rC+intrad; ++r)
    for(unsigned int k = kC-intrad; k <= kC+intrad; ++k)
    {
      GLfloat interPdist =  exp(-((span*((GLfloat)r) - x) * (span*((GLfloat)r) - x) +
                                  (span*((GLfloat)k) - z) * (span*((GLfloat)k) - z)) / (radius*radius));
      unsigned int index = r*levelSize + k;
      GLfloat oheight = heights[offset + index];

      interPdist = oheight * (1-interPdist)
                  + height * interPdist;
      
      if (oheight > interPdist)
      {
        heights[offset + index] =    oheight * hardness[index] + 
                                  interPdist * (1-hardness[index]);
        hardness[index] += HARDENINGFACTOR*(oheight - heights[offset + index]);
        if (hardness[index] > 1)
          hardness[index] = 1;
      }
    }
  recalcVertNorm(rC, kC, intrad+1);
}



void Ground::recalcVertNorm(unsigned int rC, unsigned int kC, unsigned int radius)
{
  unsigned int levelSize = (2 << (levelCount -1)) + 1;

  for (int level = levelCount-1; level >= 0; --level)
  {
    unsigned int offset    = levelIndexOffsets[level];
    levelSize = (2 << (level)) + 1;
  
    // verticies
    for(unsigned int r = rC-radius; r <= rC+radius; ++r)
      for(unsigned int k = kC-radius; k <= kC+radius; ++k)
      {
        vertices[offset + r*levelSize + k][1] = heights[offset + r*levelSize + k];
      }     
    
    // decimera ner till nästa level
    if (level != 0)
    {
      convDS(&heights[offset], levelSize, lpFilt, LPFILTRADIUS, &heights[levelIndexOffsets[level-1]], 
             rC-radius, kC-radius, rC+radius, kC+radius);
    }    

    // normals
    conv(&heights[offset], levelSize, ddrFilt, LPFILTRADIUS, dheightdr, rC-radius, kC-radius, rC+radius, kC+radius); 
    conv(&heights[offset], levelSize, ddkFilt, LPFILTRADIUS, dheightdk, rC-radius, kC-radius, rC+radius, kC+radius); 
    
    for(unsigned int r = rC-radius; r <= rC+radius; ++r)
      for(unsigned int k = kC-radius; k <= kC+radius; ++k)
        {
          normals[offset + r*levelSize + k][0] = dheightdr[r*levelSize + k];
          normals[offset + r*levelSize + k][1] = levelSpan[level];
          normals[offset + r*levelSize + k][2] = dheightdk[r*levelSize + k];
        }

    for(unsigned int r = rC-radius; r <= rC+radius; ++r)
      for(unsigned int k = kC-radius; k <= kC+radius; ++k)
      {
        GLfloat normlen = sqrt( normals[offset + r*levelSize + k][0] * normals[offset + r*levelSize + k][0] + 
                                normals[offset + r*levelSize + k][1] * normals[offset + r*levelSize + k][1] +
                                normals[offset + r*levelSize + k][2] * normals[offset + r*levelSize + k][2]);
              
        normals[offset + r*levelSize + k][0] = normals[offset + r*levelSize + k][0]/normlen;
        normals[offset + r*levelSize + k][1] = normals[offset + r*levelSize + k][1]/normlen;
        normals[offset + r*levelSize + k][2] = normals[offset + r*levelSize + k][2]/normlen;
      } 

    rC = (rC)/2;
    kC = (kC)/2;
    radius = radius/2+1;
  }
}



void Ground::conv(GLfloat f[], unsigned int maxfI, GLfloat kern[], unsigned int maxkernI,
                  GLfloat output[], unsigned int rmin, unsigned int kmin, unsigned int rmax, unsigned int kmax)
{
  unsigned int kernWid = 2*maxkernI +1;
  int sumRmin, sumKmin, sumRmax, sumKmax;
  for (unsigned int rC = rmin; rC <= rmax; ++rC)
  {
    if (rC >=  maxkernI)
      sumRmin = 0;
    else
      sumRmin = maxkernI - rC;

    if ((rC + maxkernI) < maxfI)
      sumRmax = 2*maxkernI;
    else
      sumRmax = maxfI - rC - 1 + maxkernI;

    for (unsigned int kC = kmin; kC <= kmax; ++kC)
    {
      if (kC >=  maxkernI)
        sumKmin = 0;
      else
        sumKmin = maxkernI - kC;

      if ((kC + maxkernI) < maxfI)
        sumKmax = 2*maxkernI;
      else
        sumKmax = maxfI - kC - 1 + maxkernI;

      unsigned int outindex = rC*maxfI + kC;
      output[outindex] = 0;
      for (int r = sumRmin; r <= sumRmax; ++r)
        for (int k = sumKmin; k <= sumKmax; ++k)
          output[outindex] += f[(rC+r-maxkernI)*maxfI + (kC+k-maxkernI)] * kern[r*kernWid + k];
    }
  }
}


void Ground::convDS(GLfloat f[], unsigned int maxfI, GLfloat kern[], unsigned int maxkernI,
                    GLfloat output[], unsigned int rmin, unsigned int kmin, unsigned int rmax, unsigned int kmax)
{
  unsigned int kernWid = 2*maxkernI +1;
  unsigned int maxfIL  = (maxfI+1)/2;
  int sumRmin, sumKmin, sumRmax, sumKmax;
  
  rmin = rmin + (rmin % 2); // endast jämna index intressanta
  kmin = kmin + (kmin % 2);  
  
  for (unsigned int rC = rmin; rC <= rmax; rC += 2)
  {
    if (rC >=  maxkernI)
      sumRmin = 0;
    else
      sumRmin = maxkernI - rC;

    if ((rC + maxkernI) < maxfI)
      sumRmax = 2*maxkernI;
    else
      sumRmax = maxfI - rC - 1 + maxkernI;

    for (unsigned int kC = kmin; kC <= kmax; kC += 2)
    {
      if (kC >=  maxkernI)
        sumKmin = 0;
      else
        sumKmin = maxkernI - kC;

      if ((kC + maxkernI) < maxfI)
        sumKmax = 2*maxkernI;
      else
        sumKmax = maxfI - kC - 1 + maxkernI;

      unsigned int outindex = (rC/2)*maxfIL + (kC/2);
      output[outindex] = 0;
      for (int r = sumRmin; r <= sumRmax; ++r)
        for (int k = sumKmin; k <= sumKmax; ++k)
          output[outindex] += f[(rC+r-maxkernI)*maxfI + (kC+k-maxkernI)] * kern[r*kernWid + k];
    }
  }
}
