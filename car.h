// SnowRacer
// Kristoffer 2009-04

// car.h

#ifndef CAR_H
#define CAR_H

// antal bilar i car.config
#define CAR_COUNT 4

#define PI 3.1416
#include <GL/glut.h>
class Ground;
class Sysdep;
class Camera;
class Tyre;
class ContactPoint;
struct Model;


class Car {
public:
             Car(Ground* theGround, Camera* theCamera);
             Car(Ground* theGround, Camera* theCamera, int carNo);
             Car(Ground* theGround, Camera* theCamera, int carNo, GLfloat x, GLfloat z, GLfloat phi);
             ~Car();
       
  void       switchDriving(unsigned char wheel); // byter hjul mellan drivande/ej
  void       reset();  // återställer bilen
  void       move(GLfloat x, GLfloat y, GLfloat z); // flytta bilen (används om bilen kommer utanför banan)
       
  void       Animate(); // animerar bilan
  void       SetSpeed(GLfloat steering, GLfloat acceleration); // sätt styr- och gasvärden
  void       InformCamera(); // informerar kameran om bilposition (innan draw)
  void       Draw(); // rita bilen
  void       SetHeadlight();  // ange ljuskällornas lägen
  void       getPosition(GLfloat& x, GLfloat& z, GLfloat& phi); //bilens läge
       
       
private:
  void    CarInit(Ground* theGround, Camera* theCamera, int carNo, GLfloat x, GLfloat z, GLfloat phi);
  void    loadParameters(int carNo); // ladda parametrar (definierad i car.config)
  void    DoAnimate(GLfloat dt);  //egentliga animeringsrutinen

  void    Car2World4f(GLfloat input[], GLfloat output[]);  // basbyten
  void    World2Car4f(GLfloat input[], GLfloat output[]);
  
  void    Car2World3f(GLfloat input[], GLfloat output[]);
  void    World2Car3f(GLfloat input[], GLfloat output[]);
  
  void    UpdateMatrixes();  //uppdaterar bilmatris (model-matrix)

  Tyre** tyres; // lista med pekare till däcken
  GLubyte*  tyreTexture;
          
  GLfloat position[4];
  GLfloat posdot[3];
  
  GLfloat phi;
  GLfloat phidot;
  GLfloat th;
  GLfloat thdot;
  GLfloat v;
  GLfloat vdot;
  
  GLfloat engineF;  //motorpådrag
  GLfloat steerAngle;

  GLfloat carMatrix[16];  // model-matrix
  
  Ground* theGround;    
  Camera* theCamera;
  
  int     statPrintCount; // för utskrift med jämna mellanrum
  GLfloat fpsMin;
  GLfloat fpsMax;
  int     maxSPF;
  
  Sysdep* sysdep; //systemberoende delar (tid)
      
      
  // bilparametrar
  GLfloat length;
  GLfloat width;
  GLfloat height;
  unsigned char tyreCount;
  GLfloat tyreSpringConst;
  GLfloat tyreSpringDampConst;
  GLfloat tyreSpringFullLen;
  GLfloat tyreSpringMaxSprung;
  GLfloat tyreSpringMinSprung;
  GLfloat tyreBelowGroundSpringConst;

  GLfloat tyreGroundFriction;
  GLfloat tyreGroundDamping;
  
  unsigned char conpointCount;
  ContactPoint** conpoints;

  GLfloat carInertPhi;
  GLfloat carInertTh;
  GLfloat carInertV;

  GLfloat carMass;
  GLfloat gForce;

  GLfloat airResCoeff;
  GLfloat airResCoeffrot;

  char    SoftGround;
  GLfloat GroundSoftness;
  
  // car model parameters
  Model* model;
  GLfloat modelScale;
  GLfloat modelX; // car model offset
  GLfloat modelY;
  GLfloat modelZ;
};



#endif
