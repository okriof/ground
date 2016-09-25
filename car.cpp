// SnowRacer
// Kristoffer 2009-04

// car.cpp

#include <cmath>
#include <cstdlib>
#include "car.h"
#include "tyre.h"
#include "ground.h"
#include "camera.h"
#include "sysdep.h"
#include "helpers.h"

#include "car.config" // innehåller loadParameters(int carNo)
#include <iostream>




Car::Car(Ground* theGround_p, Camera* theCamera_p)
{
  CarInit(theGround_p, theCamera_p, 0, 20, 20, PI);
}

Car::Car(Ground* theGround_p, Camera* theCamera_p, int carNo)
{
  CarInit(theGround_p, theCamera_p, carNo, 20, 20, PI);
}

Car::Car(Ground* theGround_p, Camera* theCamera_p, int carNo, GLfloat x, GLfloat z, GLfloat phiSt)
{
  CarInit(theGround_p, theCamera_p, carNo, x, z, phiSt);
}


void Car::CarInit(Ground* theGround_p, Camera* theCamera_p, int carNo, GLfloat x, GLfloat z, GLfloat phiSt)
{
  theGround = theGround_p;
  theCamera = theCamera_p;
  model = 0;
    
  // ladda data
  loadParameters(carNo);
   
  position[0] = x;
  position[2] = z;
  position[3] = 1;
  position[1] = theGround->Height(position)+3;
   
  posdot[0] = 0;
  posdot[1] = 0;
  posdot[2] = 0;
   
  phi = phiSt;
  th = 0;
  v =  0;
    
  phidot = 0;
  thdot = 0;
  vdot = 0;
    
  engineF = 0;
    
  for(unsigned char r = 0; r < 16; r++)
    carMatrix[r] = 0;
  carMatrix[15] = 1;

  UpdateMatrixes();
   
  // däck
  // textur
  tyreTexture = new GLubyte[16*16*3];
  for (int a = 0; a < 16*16*3; ++a)
    tyreTexture[a] = rand() % 256;
        
  //glGenTextures(1, &tyreTexNum);
  //glBindTexture(GL_TEXTURE_2D, tyreTexNum);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0,
	             GL_RGB, GL_UNSIGNED_BYTE, tyreTexture);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
  // contact-points
  conpointCount = tyreCount;
  conpoints = new ContactPoint*[conpointCount];
    
  for(unsigned char r = 0; r < conpointCount; ++r)
    conpoints[r] = new ContactPoint();
    
  // nollställ däckparametrar
  for(int i = 0; i < tyreCount; i++)
  {
    Car2World4f(tyres[i]->position, tyres[i]->oldWorldPos);
    tyres[i]->wSpeed = 0;
    tyres[i]->wRotation = 0;
    tyres[i]->wSteerAngle = 0;
    for(int j = 0; j < 4; ++j)
      conpoints[i]->position[j] = tyres[i]->position[j];
        
    conpoints[i]->position[1] = height;
    Car2World4f(conpoints[i]->position, conpoints[i]->oldWorldPos);
  }
    
  statPrintCount = 0;
  fpsMin = 999;
  fpsMax = 0;
  maxSPF = 0;
    
    
  // headlights
  glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 14);
  glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 90);
  glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0);
  glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, .005);
  glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, .00005);

  glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 14);
  glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 90);
  glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0);
  glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, .005);
  glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, .00005);
  
  //  "kupébelysning"
  glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 1.0);
  glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, .005);
  glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, .1);    
    
  sysdep = Sysdep::Instance();
  sysdep->getElapsedTime(); // reset counter
}

Car::~Car()
{
  for(unsigned char r = 0; r < tyreCount; ++r)
    delete tyres[r];
  delete[] tyres;
  
  for(unsigned char r = 0; r < conpointCount; ++r)
    delete conpoints[r];
  delete[] conpoints;
  delete[] tyreTexture;
  
  if (model != 0)
  {
    free(model->vertexArray);
    free(model->texCoordArray);
    free(model->colorArray);
    free(model->indexArray);
    free(model);
    model = 0;
  }
}



void Car::switchDriving(unsigned char wheel)
{
  if (wheel < tyreCount)
    tyres[wheel]->driving = ! tyres[wheel]->driving;
}



void Car::getPosition(GLfloat& x, GLfloat& z, GLfloat& phiSt)
{
  x = position[0];
  z = position[2];
  phiSt = phi;
}



void Car::SetSpeed(GLfloat steering, GLfloat acceleration)
{
  engineF = acceleration;
     
  for (unsigned char r = 0; r < tyreCount; r++)
    tyres[r]->wSteerAngle = steering * tyres[r]->steerCoefficient;
}



void Car::Animate()
{
  // tidssteg sedan förra animate
  GLfloat dt = sysdep->getElapsedTime();

  if (fpsMax < 1/dt)
    fpsMax = 1/dt;
  if (fpsMin > 1/dt)
    fpsMin = 1/dt;
    
  dt *=1.5;
    
       
  if (++statPrintCount >= 100)
  {
    printf("FPS min: %1.1f \t FPS max: %1.1f\t StepsPerFrame: %d \n", fpsMin, fpsMax, maxSPF);
    fpsMax = 0;
    fpsMin = 999;
    statPrintCount = 0;
    maxSPF = 0;
  }
     
  int spf = 1;
  while (dt > 0.0125)
  {
    dt -= .01;
    DoAnimate(.01);
    ++spf;
  }
  DoAnimate(dt);
     
  if (maxSPF < spf)
    maxSPF = spf;
}



void Car::move(GLfloat x, GLfloat y, GLfloat z)
{
  printf("Moved...\n");
  position[0] += x;
  position[1] += y;
  position[2] += z;
  
  UpdateMatrixes();
  
  for(int t = 0; t < tyreCount; ++t)
  {
    tyres[t]->oldWorldPos[0] += x;
    tyres[t]->oldWorldPos[1] += y;
    tyres[t]->oldWorldPos[2] += z;

    conpoints[t]->oldWorldPos[0] += x;
    conpoints[t]->oldWorldPos[1] += x;
    conpoints[t]->oldWorldPos[2] += x;
  }
}

void Car::reset()
{
  printf("Reset...\n");
  position[1] = theGround->Height(position)+4;
    
  posdot[0] = 0;
  posdot[1] = 0;
  posdot[2] = 0;
    
  th = 0;
  v  = 0;
  phidot = 0;
  thdot  = 0;
  vdot   = 0;
    
  UpdateMatrixes();
    
  for(int t = 0; t < tyreCount; ++t)
  {
    tyres[t]->position[1] = -tyreSpringMaxSprung;
    Car2World4f(tyres[t]->position, tyres[t]->oldWorldPos);
    tyres[t]->wSpeed = 0;
    tyres[t]->wRotation = 0;
    tyres[t]->wSteerAngle = 0;
    Car2World4f(conpoints[t]->position, conpoints[t]->oldWorldPos);
  }
}



void Car::DoAnimate(GLfloat dt)
{
  // fysiksimulering.. 
  UpdateMatrixes(); // uppdatera transformationsmatriser
    
  GLfloat tyreworld[4];
  GLfloat tyretemp[4];
  
  GLfloat groundHeight;
  GLfloat groundNormalWorld[3];
  GLfloat groundNormalCar[3];
  GLfloat upWorld[3] = {0, 1, 0};
  GLfloat upCar[3];
  World2Car3f(upWorld, upCar);
    
  char    suspensionState; // 0 fullt ute, 1 normal, 2 i botten
  GLfloat suspensionCompressionLen;
  GLfloat tyreNormF; // normalkraft
  GLfloat tyreTanF; // tangentialkraft (storlek)
  GLfloat tyreF[3];  //kraft från marken på däcket 
  GLfloat tyreFtemp[3]; // temp-kraft-variabel
  GLfloat totCgF[3] = {0.0, 0.0, 0.0};  //total kraft på Cg (i bilkoordinatsystemet)
  GLfloat totCgM[3] = {0.0, 0.0, 0.0};  //moment i CG
    
  // tyre forces  
  for(int t = 0; t < tyreCount; ++t) 
  {
    tyreF[0] = 0;
    tyreF[1] = 0;
    tyreF[2] = 0;
          
    // update position
    Car2World4f(tyres[t]->position, tyreworld); // tyre pos in world coords
        
    //looopa och uppdatera pos
    for (int a = 0; a < 2; ++a)
    {
      groundHeight = theGround->Height(tyreworld); // ground height at tyre pos. (bygger på liten förflyttning i x- och z-led)
      tyreworld[1] = groundHeight;   
      World2Car4f(tyreworld, tyretemp); // ny däckspos. i bilkoord.
      tyretemp[0] = tyres[t]->position[0]; // kan ej röra sig i x- eller z-led
      tyretemp[2] = tyres[t]->position[2];
          
      // kolla fjädring (återställ däcksposition inom fjädringsvägen)
      if ((upCar[1] < 0) || (tyretemp[1] < (-tyreSpringMaxSprung))) // clamp i y-led
      {
        suspensionCompressionLen = 0;
        tyretemp[1] = -tyreSpringMaxSprung;
        suspensionState = 0; // fullt ute
      }
      else if (tyretemp[1] > -tyreSpringMinSprung)
      {
        suspensionCompressionLen = (tyreSpringFullLen - (-tyretemp[1]));
        tyretemp[1] = -tyreSpringMinSprung;
        suspensionState = 2; // i botten
      }
      else
      {
        suspensionCompressionLen = (tyreSpringFullLen - (-tyretemp[1]));
        suspensionState = 1; // normalläge
      }
          
      Car2World4f(tyretemp, tyreworld); // uppdatera världsposition
    } // slut (uppdatera hjulpos)
        
    groundHeight = theGround->Height(tyreworld);
    theGround->Normal(tyreworld[0], tyreworld[2], groundNormalWorld); // marknormal vid position för däcket
    World2Car3f(groundNormalWorld, groundNormalCar); // marknormal i bilkoordinatsystem
    
    if (tyreworld[1] <= groundHeight+.001) // kontakt med marken
    {
      tyreF[1] = suspensionCompressionLen * tyreSpringConst;
      
      if (suspensionState == 1)
        tyreF[1] -= (tyres[t]->position[1] - tyretemp[1])/dt * tyreSpringDampConst; // dpos/dt, dämpningskraft
       
      // friktion mot marken
      GLfloat tyreWorldVel[3]; // däckhastighet i världskoord
      GLfloat tyreCarVel[3];   // däckhastighet mot marken i bilkoordinater
      GLfloat thrustForceNormal[3]; // gas-riktning
      GLfloat tyreNormalComp; // däckhastighet skalärt ytnormal
      
      GLfloat suspensionNormalLen = tyreF[1] * groundNormalCar[1]; // enda nollskilda elementet i tyreF (skalärprod)
      
      if (suspensionNormalLen > 0)
      {      
        // sätt normaldelen.
        tyreF[0] += groundNormalCar[0] * suspensionNormalLen;
        tyreF[1] += groundNormalCar[1] * suspensionNormalLen;
        tyreF[2] += groundNormalCar[2] * suspensionNormalLen;
      }
      
      // normalkraft längs ytnormalen
      tyreF[0] += groundNormalCar[0] * (groundHeight - tyreworld[1]) * tyreBelowGroundSpringConst;
      tyreF[1] += groundNormalCar[1] * (groundHeight - tyreworld[1]) * tyreBelowGroundSpringConst;
      tyreF[2] += groundNormalCar[2] * (groundHeight - tyreworld[1]) * tyreBelowGroundSpringConst;
      
              
      // om bilen nära att passera däckomslagspunkten, undertryck förflyttningskrafter
      if ((fabs(cos(th)) < .1) || (fabs(cos(v)) < .1))
      {
        tyres[t]->oldWorldPos[0] = tyreworld[0];
        tyres[t]->oldWorldPos[1] = tyreworld[1];
        tyres[t]->oldWorldPos[2] = tyreworld[2];
        tyres[t]->oldWorldPos[3] = tyreworld[3];
      }
                  
      tyreWorldVel[0] = (tyreworld[0] - tyres[t]->oldWorldPos[0]) / dt;
      tyreWorldVel[1] = (tyreworld[1] - tyres[t]->oldWorldPos[1]) / dt;
      tyreWorldVel[2] = (tyreworld[2] - tyres[t]->oldWorldPos[2]) / dt;
              
      tyreNormalComp = tyreWorldVel[0]*groundNormalWorld[0] +
                       tyreWorldVel[1]*groundNormalWorld[1] +
                       tyreWorldVel[2]*groundNormalWorld[2];
      
       // plocka bort normalkomponent
      tyreWorldVel[0] -= tyreNormalComp*groundNormalWorld[0];
      tyreWorldVel[1] -= tyreNormalComp*groundNormalWorld[1];
      tyreWorldVel[2] -= tyreNormalComp*groundNormalWorld[2];
    
              
      World2Car3f(tyreWorldVel, tyreCarVel);
              
      // styrning och drivning..
      // kryssa marknormalen med hjulaxelnormalen för att få gasriktningsnormalen
      GLfloat alpha = tyres[t]->wSteerAngle;
              
      thrustForceNormal[0] = groundNormalCar[1] * sin(alpha);
      thrustForceNormal[1] = groundNormalCar[2] * cos(alpha) - groundNormalCar[0] * sin(alpha);
      thrustForceNormal[2] =                                 - groundNormalCar[1] * cos(alpha);
      // normalisera
      GLfloat thrustForceNormalLen = sqrt( thrustForceNormal[0]*thrustForceNormal[0] +
                                           thrustForceNormal[1]*thrustForceNormal[1] +
                                           thrustForceNormal[2]*thrustForceNormal[2] );          
              
      thrustForceNormal[0] /= thrustForceNormalLen;
      thrustForceNormal[1] /= thrustForceNormalLen;
      thrustForceNormal[2] /= thrustForceNormalLen;
              
      // beräkna periferihastighet hos hjulet
      tyres[t]->wSpeed = (tyreCarVel[0]*thrustForceNormal[0] +
                          tyreCarVel[1]*thrustForceNormal[1] +
                          tyreCarVel[2]*thrustForceNormal[2]) *.97;
              
      if (tyres[t]->driving) // om drivhjul
      {
        tyres[t]->wSpeed = -engineF;
        tyreCarVel[0] += engineF*thrustForceNormal[0]; 
        tyreCarVel[1] += engineF*thrustForceNormal[1]; 
        tyreCarVel[2] += engineF*thrustForceNormal[2]; 
      }
      else // annars nollställ kraften i den riktningen
      {
        thrustForceNormalLen = tyreCarVel[0]*thrustForceNormal[0] + // skalärt mot thrustnormalen
                               tyreCarVel[1]*thrustForceNormal[1] +
                               tyreCarVel[2]*thrustForceNormal[2];    
        tyreCarVel[0] -= thrustForceNormalLen*thrustForceNormal[0];
        tyreCarVel[1] -= thrustForceNormalLen*thrustForceNormal[1];
        tyreCarVel[2] -= thrustForceNormalLen*thrustForceNormal[2];
      }
                
      // lägg till kraft till total däckkraft
      tyreF[0] += -tyreCarVel[0]*tyreGroundDamping;
      tyreF[1] += -tyreCarVel[1]*tyreGroundDamping;
      tyreF[2] += -tyreCarVel[2]*tyreGroundDamping;
    }  // slut (hjulet i kontakt med marken)
    
            
    tyres[t]->position[1] = tyretemp[1]; // uppdatera hjulposition
    tyres[t]->oldWorldPos[0] = tyreworld[0];
    tyres[t]->oldWorldPos[1] = tyreworld[1];
    tyres[t]->oldWorldPos[2] = tyreworld[2];
    tyres[t]->oldWorldPos[3] = tyreworld[3];
            
    // skalärt marknormalen ger normalkraften från marken på däcken
    tyreNormF = tyreF[0]*groundNormalCar[0] +
                tyreF[1]*groundNormalCar[1] +
                tyreF[2]*groundNormalCar[2]; 
                        
    if (tyreNormF > 0) // om normalen positiv, använd kraften
    {
      tyreFtemp[0] = tyreNormF*groundNormalCar[0]; // normalkomponent
      tyreFtemp[1] = tyreNormF*groundNormalCar[1];
      tyreFtemp[2] = tyreNormF*groundNormalCar[2];
            
      tyreF[0] -= tyreFtemp[0];  //  tangentialkomponent (ta bort normalkomponent)
      tyreF[1] -= tyreFtemp[1];
      tyreF[2] -= tyreFtemp[2];
            
      tyreTanF = sqrt( tyreF[0]*tyreF[0] + // magnitud av tangentialkomponent
                       tyreF[1]*tyreF[1] +
                       tyreF[2]*tyreF[2]);
                                
      // om för stor i förhållande till vad friktionen tillåter, korta av
      if (tyreTanF > tyreNormF * tyreGroundFriction)
      {
        tyreF[0] *= (tyreNormF * tyreGroundFriction / tyreTanF);
        tyreF[1] *= (tyreNormF * tyreGroundFriction / tyreTanF);
        tyreF[2] *= (tyreNormF * tyreGroundFriction / tyreTanF);
      }
       
           
      tyreF[0] += tyreFtemp[0];  //  lägg till normalkomponent igen
      tyreF[1] += tyreFtemp[1];
      tyreF[2] += tyreFtemp[2];
            
            
      // lägg till till hjulkraft till totalkraft
      totCgF[0] += tyreF[0];
      totCgF[1] += tyreF[1];
      totCgF[2] += tyreF[2];
            
      //förflyttningsmoment (kryssprodukt...)
      totCgM[0] += tyres[t]->position[1] * tyreF[2] -
                   tyres[t]->position[2] * tyreF[1];
      totCgM[1] += tyres[t]->position[2] * tyreF[0] -
                   tyres[t]->position[0] * tyreF[2];
      totCgM[2] += tyres[t]->position[0] * tyreF[1] -
                   tyres[t]->position[1] * tyreF[0];
                             
      // mjuk mark, nertryckt av normalkraft
      if (SoftGround)
        theGround->setHeight(tyreworld[0], tyreworld[2], tyreworld[1] -GroundSoftness*dt*tyreNormF, tyres[t]->radius);
    } //slut (kraft uppåt)           
    
    // rotera hjulet (animering)
    if (tyres[t]->driving) // om drivhjul
      tyres[t]->wSpeed = -engineF;
    tyres[t]->wRotation += tyres[t]->wSpeed/tyres[t]->radius*dt;
  } // slut loop över alla däck
    

    
    
  // contactPoint forces
  for(int t = 0; t < conpointCount; ++t)
  {
    tyreF[0] = 0;
    tyreF[1] = 0;
    tyreF[2] = 0;
          
    // update position
    Car2World4f(conpoints[t]->position, tyreworld); // conpoint pos in world coords
           
    theGround->Normal(tyreworld[0], tyreworld[2], groundNormalWorld); // marknormal vid position för point
    World2Car3f(groundNormalWorld, groundNormalCar); // marknormal i bilkoordinatsystem
    groundHeight = theGround->Height(tyreworld);
    
    if (tyreworld[1] <= groundHeight+.001) // kontakt med marken
    {
      GLfloat  wdotCar[3] = {cos(v)*thdot + sin(v)*cos(th)*phidot, -sin(v)*thdot +cos(v)*cos(th)*phidot, -sin(th)*phidot + vdot};
      GLfloat  CgdotCar[3];
      GLfloat* p = conpoints[t]->position; 
      World2Car3f(posdot, CgdotCar); 
      
      // hastighet i kontaktpunkten
      GLfloat pdot[3] = {wdotCar[1]*p[2]-wdotCar[2]*p[1] + CgdotCar[0],
                         wdotCar[2]*p[0]-wdotCar[0]*p[2] + CgdotCar[1],
                         wdotCar[0]*p[1]-wdotCar[1]*p[0] + CgdotCar[2]};
     
      GLfloat pdotNormal = pdot[0]*groundNormalCar[0] + pdot[1]*groundNormalCar[1] + pdot[2]*groundNormalCar[2];
      // normalkraft längs ytnormalen (se om bilen på väg neråt)
      if ((pdotNormal < 0))
      {
        tyreF[0] += groundNormalCar[0] * (groundHeight - tyreworld[1]) * tyreBelowGroundSpringConst;
        tyreF[1] += groundNormalCar[1] * (groundHeight - tyreworld[1]) * tyreBelowGroundSpringConst;
        tyreF[2] += groundNormalCar[2] * (groundHeight - tyreworld[1]) * tyreBelowGroundSpringConst;
      }      
        
      // friktion mot marken
      GLfloat tyreWorldVel[3]; // pointhastighet i världskoord
      GLfloat tyreCarVel[3];   // pointhastighet mot marken i bilkoordinater
      GLfloat tyreNormalComp; // pointhastighet skalärt ytnormal
                  
      tyreWorldVel[0] = (tyreworld[0] - conpoints[t]->oldWorldPos[0]) / dt;
      tyreWorldVel[1] = (tyreworld[1] - conpoints[t]->oldWorldPos[1]) / dt;
      tyreWorldVel[2] = (tyreworld[2] - conpoints[t]->oldWorldPos[2]) / dt;
              
      tyreNormalComp = tyreWorldVel[0]*groundNormalWorld[0] +
                       tyreWorldVel[1]*groundNormalWorld[1] +
                       tyreWorldVel[2]*groundNormalWorld[2];
      
       // plocka bort normalkomponent
      tyreWorldVel[0] -= tyreNormalComp*groundNormalWorld[0];
      tyreWorldVel[1] -= tyreNormalComp*groundNormalWorld[1];
      tyreWorldVel[2] -= tyreNormalComp*groundNormalWorld[2];
    
              
      World2Car3f(tyreWorldVel, tyreCarVel);
    
      // lägg till kraft till total pointkraft
      tyreF[0] += -tyreCarVel[0]*tyreGroundDamping;
      tyreF[1] += -tyreCarVel[1]*tyreGroundDamping;
      tyreF[2] += -tyreCarVel[2]*tyreGroundDamping;
    }  // slut (point i kontakt med marken)
    
    conpoints[t]->oldWorldPos[0] = tyreworld[0];
    conpoints[t]->oldWorldPos[1] = tyreworld[1];
    conpoints[t]->oldWorldPos[2] = tyreworld[2];
            
    // skalärt marknormalen ger normalkraften från marken
    tyreNormF = tyreF[0]*groundNormalCar[0] +
                tyreF[1]*groundNormalCar[1] +
                tyreF[2]*groundNormalCar[2]; 
                        
    if (tyreNormF > 0) // om normalen positiv, använd kraften
    {
      tyreFtemp[0] = tyreNormF*groundNormalCar[0]; // normalkomponent
      tyreFtemp[1] = tyreNormF*groundNormalCar[1];
      tyreFtemp[2] = tyreNormF*groundNormalCar[2];
            
      tyreF[0] -= tyreFtemp[0];  //  tangentialkomponent (ta bort normalkomponent)
      tyreF[1] -= tyreFtemp[1];
      tyreF[2] -= tyreFtemp[2];
            
      tyreTanF = sqrt( tyreF[0]*tyreF[0] + // magnitud av tangentialkomponent
                       tyreF[1]*tyreF[1] +
                       tyreF[2]*tyreF[2]); 
                       
                                
      // om för stor i förhållande till vad friktionen tillåter, korta av
      if (tyreTanF > tyreNormF * tyreGroundFriction)
      {
        tyreF[0] *= (tyreNormF * tyreGroundFriction / tyreTanF);
        tyreF[1] *= (tyreNormF * tyreGroundFriction / tyreTanF);
        tyreF[2] *= (tyreNormF * tyreGroundFriction / tyreTanF);
      }
       
           
      tyreF[0] += tyreFtemp[0];  //  lägg till normalkomponent igen
      tyreF[1] += tyreFtemp[1];
      tyreF[2] += tyreFtemp[2];
            
            
      // lägg till till hjulkraft till totalkraft
      totCgF[0] += tyreF[0];
      totCgF[1] += tyreF[1];
      totCgF[2] += tyreF[2];
            
      //förflyttningsmoment (kryssprodukt...)
      totCgM[0] += conpoints[t]->position[1] * tyreF[2] -
                   conpoints[t]->position[2] * tyreF[1];
      totCgM[1] += conpoints[t]->position[2] * tyreF[0] -
                   conpoints[t]->position[0] * tyreF[2];
      totCgM[2] += conpoints[t]->position[0] * tyreF[1] -
                   conpoints[t]->position[1] * tyreF[0];
                             
    } //slut (kraft uppåt)           
  } // slut loop över alla contactpoints
    
    
  Car2World3f(totCgF, tyreF); //tyreF som tempvariabel
  // gör om totCgF till världskoordinatsystemet
    
  posdot[0] += ( tyreF[0]          - fabs(posdot[0])*posdot[0]*airResCoeff )/carMass *dt;
  posdot[1] += ( tyreF[1] - gForce - fabs(posdot[1])*posdot[1]*airResCoeff )/carMass *dt;
  posdot[2] += ( tyreF[2]          - fabs(posdot[2])*posdot[2]*airResCoeff )/carMass *dt;
   
  position[0] += posdot[0] * dt;
  position[1] += posdot[1] * dt;
  position[2] += posdot[2] * dt;
    
  // vinklar o greger
  GLfloat totCgMth[3] = {cos(v)*totCgM[0]-sin(v)*totCgM[1], sin(v)*totCgM[0]+cos(v)*totCgM[1], totCgM[2]};
  //GLfloat totCgMphi[3] = {totCgMth[0], cos(th)*totCgMth[1]-sin(th)*totCgMth[2], sin(th)*totCgMth[1]+cos(th)*totCgMth[2]};
  GLfloat totCgMphi = cos(th)*totCgMth[1]-sin(th)*totCgMth[2];

  vdot   += ( totCgM[2] - fabs(vdot  )*vdot  *airResCoeffrot ) /carInertV   * dt;  
  phidot += ( totCgMphi - fabs(phidot)*phidot*airResCoeffrot ) /carInertPhi * dt;
  thdot  += ( totCgMth[0] - fabs(thdot )*thdot *airResCoeffrot ) /carInertTh  * dt;

    
  phi += phidot * dt;
  th  += thdot  * dt;
  v   += vdot   * dt;
}



void Car::InformCamera()
{  
  theCamera->informCarPosition(position, phi, th, v); 
};



void Car::Draw()
{
  glPushMatrix();
  glMultMatrixf(carMatrix);
    
  for(unsigned char t = 0; t < tyreCount; t++)
    tyres[t]->Draw();
    
  if (! model)
  {
    GLfloat emiss[] = {.1,.1,.5,1};
    glMaterialfv(GL_FRONT, GL_EMISSION, emiss); 
    glBegin(GL_TRIANGLES);
    glNormal3f(0,1,0);
    glVertex3f(-width/2,0,length/2);
    glVertex3f(width/2,0,length/2);
    glVertex3f(0,0,-length/2);
    glEnd();
  
    emiss[1] = .5;
    emiss[2] = .1;
    glMaterialfv(GL_FRONT, GL_EMISSION, emiss); 
    glBegin(GL_TRIANGLES);
    glNormal3f(0,-1,0);
    glVertex3f(-width/2,0,length/2);
    glVertex3f(0,0,-length/2);
    glVertex3f(width/2,0,length/2);
    glEnd();
  }
  else
  {
    glTranslatef(modelX, modelY, modelZ);
    glScalef(modelScale, modelScale, modelScale);
    glEnable(GL_NORMALIZE);
    glVertexPointer(3, GL_FLOAT, 0, model->vertexArray);
    glNormalPointer(GL_FLOAT, 0, model->normalArray);
    glDrawElements(GL_TRIANGLES, model->numIndices, GL_UNSIGNED_INT, model->indexArray);
    glDisable(GL_NORMALIZE);    
  }
  
  glPopMatrix();
}



void Car::SetHeadlight()
{
  glPushMatrix();
  glMultMatrixf(carMatrix);
  GLfloat headlPos[4] = {-1.0, 3.0, 0.0, 1.0};
  GLfloat headlDir[3] = {-0.35, -.3, -1.0};
  glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, headlDir);
  glLightfv(GL_LIGHT1, GL_POSITION, headlPos);
  
  headlDir[0] = 0.35;
  headlPos[0] = 1.0;
  glLightfv(GL_LIGHT2, GL_POSITION, headlPos);
  glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, headlDir);
  
  headlPos[0] = 0.0;
  glLightfv(GL_LIGHT3, GL_POSITION, headlPos);
  glPopMatrix();     
}



void Car::Car2World4f(GLfloat input[], GLfloat output[])
{
    output[0] = input[0] * carMatrix[0] +
                input[1] * carMatrix[4] +
                input[2] * carMatrix[8] +
                input[3] * carMatrix[12];
    output[1] = input[0] * carMatrix[1] +
                input[1] * carMatrix[5] +
                input[2] * carMatrix[9] +
                input[3] * carMatrix[13];
    output[2] = input[0] * carMatrix[2] +
                input[1] * carMatrix[6] +
                input[2] * carMatrix[10] +
                input[3] * carMatrix[14];
    output[3] = input[0] * carMatrix[3] +
                input[1] * carMatrix[7] +
                input[2] * carMatrix[11] +
                input[3] * carMatrix[15];
}



void Car::World2Car4f(GLfloat input[], GLfloat output[])
{ // förutsätter carMatrix endast rotation + translation
    output[0] = (input[0]-carMatrix[12]) * carMatrix[0] +
                (input[1]-carMatrix[13]) * carMatrix[1] +
                (input[2]-carMatrix[14]) * carMatrix[2];
    output[1] = (input[0]-carMatrix[12]) * carMatrix[4] +
                (input[1]-carMatrix[13]) * carMatrix[5] +
                (input[2]-carMatrix[14]) * carMatrix[6];
    output[2] = (input[0]-carMatrix[12]) * carMatrix[8] +
                (input[1]-carMatrix[13]) * carMatrix[9] +
                (input[2]-carMatrix[14]) * carMatrix[10];
    output[3] = 1;
}



void Car::Car2World3f(GLfloat input[], GLfloat output[])
{
    output[0] = input[0] * carMatrix[0] +
                input[1] * carMatrix[4] +
                input[2] * carMatrix[8];
    output[1] = input[0] * carMatrix[1] +
                input[1] * carMatrix[5] +
                input[2] * carMatrix[9];
    output[2] = input[0] * carMatrix[2] +
                input[1] * carMatrix[6] +
                input[2] * carMatrix[10];
}



void Car::World2Car3f(GLfloat input[], GLfloat output[])
{   // endast rotationsmatris
    output[0] = input[0] * carMatrix[0] +
                input[1] * carMatrix[1] +
                input[2] * carMatrix[2];
    output[1] = input[0] * carMatrix[4] +
                input[1] * carMatrix[5] +
                input[2] * carMatrix[6];
    output[2] = input[0] * carMatrix[8] +
                input[1] * carMatrix[9] +
                input[2] * carMatrix[10];
}



void Car::UpdateMatrixes()
{
 //matriser
    carMatrix[0] =  cos(phi)*cos(v) + sin(phi)*sin(th)*sin(v);
    carMatrix[4] = -cos(phi)*sin(v) + sin(phi)*sin(th)*cos(v);
    carMatrix[8] =  sin(phi)*cos(th);
    
    carMatrix[1] =  cos(th)*sin(v);
    carMatrix[5] =  cos(th)*cos(v);
    carMatrix[9] =  -sin(th);
    
    carMatrix[2 ] = -sin(phi)*cos(v) + cos(phi)*sin(th)*sin(v);
    carMatrix[6 ] =  sin(phi)*sin(v) + cos(phi)*sin(th)*cos(v);
    carMatrix[10] =  cos(phi)*cos(th);
    
    carMatrix[12] = position[0];
    carMatrix[13] = position[1];
    carMatrix[14] = position[2];
}
