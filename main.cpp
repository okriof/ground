// SnowRacer
// Kristoffer 2009-04

// main.cpp

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <GL/glut.h>
#include "camera.h"
#include "ground.h"
#include "car.h"
#include "sysdep.h"
#include "demodefines.h"

using namespace std;
const GLfloat ballSpeed = 0.01; // sol/måne banvinkelhastighet
const int animstep = 20;        // 1/högsta framerate

unsigned char buttonN = 0;      // knapptryckningar
int mousex, mousey;
int viewmx, viewmy;

char timeOfDay = 0;  // states
char headlights = 0;
int carNo = 0;

Camera*    theCamera;
Ground*    theGround;
Car*       theCar;
Sysdep*    sysdep;

GLfloat ballAng = 0;
GLfloat light_pos[] = {0, 0, 0, 1};

void mousepress(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN)
  {
    if (button == GLUT_LEFT_BUTTON)
    {
      buttonN = 1;
      mousex = x;
      mousey = y;
    }
    if (button == GLUT_RIGHT_BUTTON)
    {
      buttonN = 2;
      viewmx = x;
      viewmy = y;
      theCamera->viewDirection(0, 0);
    }
  }
  else
  {
    buttonN = 0;
    theCar->SetSpeed(0, 0);
  }
}


void mousemotion(int x, int y)
{
  if (buttonN == 1)
  {
    theCar->SetSpeed(0.0025 * (x-mousex), -.03*(mousey-y));
  }
  if (buttonN == 2)
  {
    theCamera->viewDirection(0.01 * (x-viewmx), 0.005 * (y-viewmy));
  }
}


void keyboardpress(unsigned char key, int x, int y)
{
  GLfloat specdiff[] = {.04, .04, .07, 1};
  GLfloat headlightdiffuse[] = {.9, .9, .5, 1};
  GLfloat cx, cy, cphi;
  switch (key)
  { 
  case 'q': // quit
    exit(0);
    break;
  case 'c':  // byt kamera
    theCamera->changeMode();
    break;
  case 'd':  // byt sol/måne
    timeOfDay = 1-timeOfDay;
    if (timeOfDay == 1)
    {
      specdiff[0] = specdiff[1] = .7;
      specdiff[2] = .8;
    }
      
    glLightfv(GL_LIGHT0, GL_DIFFUSE, specdiff);
    break;
    
  case 'h': // strålkastare
    headlights = 1-headlights;
    if (headlights == 0)
    {
      headlightdiffuse[0]=headlightdiffuse[1]=headlightdiffuse[2]=0;
    }
    glLightfv(GL_LIGHT1, GL_DIFFUSE, headlightdiffuse);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, headlightdiffuse);
    break;
    
  case 'v': // byt bil
    if (++carNo >= CAR_COUNT) 
      carNo = 0;
    
    theCar->getPosition(cx,cy,cphi);
    delete theCar;
    theCar = new Car(theGround, theCamera, carNo,cx,cy,cphi);
    theGround->setCar(theCar);
    break;
    
  case 'r': // reset car
    theCar->reset();
    break;
    
  case '1': theCar->switchDriving(0); break;
  case '2': theCar->switchDriving(1); break;
  case '3': theCar->switchDriving(2); break;
  case '4': theCar->switchDriving(3); break;
  case '5': theCar->switchDriving(4); break;
  case '6': theCar->switchDriving(5); break;
  case '7': theCar->switchDriving(6); break;
  case '8': theCar->switchDriving(7); break;
  }
}


void display(void) 
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  theCar->InformCamera();
  
  theCamera->setCamera();    
    
  theCar->SetHeadlight();
    
  // solen/månen
  glPushMatrix();
  glRotatef(-180/PI*ballAng, 0,1,0);
  glTranslatef(0,300,100);
  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
  glPopMatrix();
  
  GLfloat emiss[] = {0,0,0,1};
  glMaterialfv(GL_FRONT, GL_EMISSION, emiss); 
    
  sysdep->setGroundDrawMode();
  theGround->Draw();
    
  sysdep->setOthersDrawMode();
  theCar->Draw();
 
  // kompass & lutnings-indikator
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  
  GLfloat compCCamera[] = {0, .5, -1, 1};
  GLfloat compCWorld[4];
  GLfloat csize = .1;
  
  theCamera->Camera2World4f(compCCamera, compCWorld);
  glColor3f(0, .8, 0);
  glBegin(GL_LINES);
  glVertex3f(compCWorld[0], compCWorld[1], compCWorld[2]);
  glVertex3f(compCWorld[0], compCWorld[1]+csize, compCWorld[2]);
  glEnd();  
  
  compCCamera[1] = -.5;
  theCamera->Camera2World4f(compCCamera, compCWorld);
    
  glBegin(GL_TRIANGLE_FAN);
  glColor3f(0, .5, .5);
  glVertex3f(compCWorld[0], compCWorld[1], compCWorld[2]);
  glColor3f(0, 0, 1);
  glVertex3f(compCWorld[0]+csize, compCWorld[1], compCWorld[2]);
  glColor3f(0, .5, .5);
  glVertex3f(compCWorld[0]+csize*.3, compCWorld[1], compCWorld[2]-csize*.3);
  glVertex3f(compCWorld[0], compCWorld[1], compCWorld[2]-csize);  
  glVertex3f(compCWorld[0]-csize*.3, compCWorld[1], compCWorld[2]-csize*.3);
  glVertex3f(compCWorld[0]-csize, compCWorld[1], compCWorld[2]);  
  glVertex3f(compCWorld[0]-csize*.3, compCWorld[1], compCWorld[2]+csize*.3);
  glColor3f(1, 0, 0);
  glVertex3f(compCWorld[0], compCWorld[1], compCWorld[2]+csize);  
  glColor3f(0, .5, .5);
  glVertex3f(compCWorld[0]+csize*.3, compCWorld[1], compCWorld[2]+csize*.3);
  glColor3f(0, 0, 1);
  glVertex3f(compCWorld[0]+csize, compCWorld[1], compCWorld[2]); 
  glEnd();
  
  glBegin(GL_TRIANGLE_FAN);
  glColor3f(.5, 0, .5);
  glVertex3f(compCWorld[0], compCWorld[1], compCWorld[2]);
  glColor3f(0, 0, 1);
  glVertex3f(compCWorld[0]+csize, compCWorld[1], compCWorld[2]);   
  glColor3f(.5, 0, .5);
  glVertex3f(compCWorld[0]+csize*.3, compCWorld[1], compCWorld[2]+csize*.3);
  glColor3f(1, 0, 0);
  glVertex3f(compCWorld[0], compCWorld[1], compCWorld[2]+csize);
  glColor3f(.5, 0, .5);
  glVertex3f(compCWorld[0]-csize*.3, compCWorld[1], compCWorld[2]+csize*.3);
  glVertex3f(compCWorld[0]-csize, compCWorld[1], compCWorld[2]);  
  glVertex3f(compCWorld[0]-csize*.3, compCWorld[1], compCWorld[2]-csize*.3);
  glVertex3f(compCWorld[0], compCWorld[1], compCWorld[2]-csize);  
  glVertex3f(compCWorld[0]+csize*.3, compCWorld[1], compCWorld[2]-csize*.3);
  glColor3f(0, 0, 1);
  glVertex3f(compCWorld[0]+csize, compCWorld[1], compCWorld[2]);


  glEnd();
  
  glBegin(GL_LINE_STRIP);
  glColor3f(0, .8, 0);
  glVertex3f(compCWorld[0]+csize, compCWorld[1], compCWorld[2]+csize);
  glVertex3f(compCWorld[0]-csize, compCWorld[1], compCWorld[2]+csize);
  glVertex3f(compCWorld[0]-csize, compCWorld[1], compCWorld[2]-csize);
  glVertex3f(compCWorld[0]+csize, compCWorld[1], compCWorld[2]-csize);
  glVertex3f(compCWorld[0]+csize, compCWorld[1], compCWorld[2]+csize);
  glEnd();
  
  glBegin(GL_LINES);
  glVertex3f(compCWorld[0]+csize*.5, compCWorld[1], compCWorld[2]+csize);
  glVertex3f(compCWorld[0]+csize*.5, compCWorld[1], compCWorld[2]-csize);
  glVertex3f(compCWorld[0]-csize*.5, compCWorld[1], compCWorld[2]+csize);
  glVertex3f(compCWorld[0]-csize*.5, compCWorld[1], compCWorld[2]-csize);
  glVertex3f(compCWorld[0]+csize, compCWorld[1], compCWorld[2]+csize*.5);
  glVertex3f(compCWorld[0]-csize, compCWorld[1], compCWorld[2]+csize*.5);
  glVertex3f(compCWorld[0]+csize, compCWorld[1], compCWorld[2]-csize*.5);
  glVertex3f(compCWorld[0]-csize, compCWorld[1], compCWorld[2]-csize*.5);
  glEnd();
  
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glutSwapBuffers();
}


void init(void)
{
  glClearColor(0,0,0,1);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHT2);
  glEnable(GL_LIGHT3);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glShadeModel(GL_SMOOTH);

  GLfloat ambientSpec[] = {.0, .0, .0, 1};
  GLfloat L3diffuse[] = {.6, .5, .4, 1};
  glLightfv(GL_LIGHT0, GL_SPECULAR, ambientSpec);
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambientSpec);
  glLightfv(GL_LIGHT1, GL_AMBIENT, ambientSpec);
  glLightfv(GL_LIGHT1, GL_SPECULAR, ambientSpec);
  glLightfv(GL_LIGHT2, GL_AMBIENT, ambientSpec);
  glLightfv(GL_LIGHT2, GL_SPECULAR, ambientSpec);
  glLightfv(GL_LIGHT3, GL_AMBIENT, ambientSpec);
  glLightfv(GL_LIGHT3, GL_SPECULAR, ambientSpec);
  glLightfv(GL_LIGHT3, GL_DIFFUSE, L3diffuse);
  keyboardpress('d', 0, 0);  // sätter L0 diffuse
  keyboardpress('h', 0, 0);  // sätter L1 och L2 diffuse

  GLfloat specdiffmtrl[] = {.8, .8, .8, 1};  //snö
  glMaterialfv(GL_FRONT, GL_DIFFUSE, specdiffmtrl);
  glMaterialfv(GL_FRONT, GL_AMBIENT, ambientSpec);
}


void animate(int ignoredValue)
{
  glutTimerFunc(animstep, animate, 0);
    
  ballAng += ballSpeed;   
  
  theCar->Animate();
    
  glutPostRedisplay();
}


void printKeyList()
{
  cout << "\nControls:\n";
  cout << "========================\n";
  cout << "Left click&hold  -- drive\n";
  cout << "Right click&hold -- look around\n";
  cout << "v   -- change vehicle\n";
  cout << "r   -- reset vehicle\n";
  cout << "h   -- headlights on/off\n";
  cout << "d   -- daylight on/off\n";
  cout << "c   -- toggle camera mode\n";
  cout << "1-8 -- toggle drive on wheel n\n" << endl;
}


int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(900,900);
  glutCreateWindow("SnowRacer3D");
  //glutGameModeString( "1024x768:32@60" );
  //glutEnterGameMode();

    
  srand(time(0));
  
  sysdep = Sysdep::Instance();
  sysdep->loadShader();
    
  init();

  theCamera = new Camera;

  // generera mark
  theGround = new Ground(WORLD_SIZE, .2, theCamera);
  theGround->RndGround(5000, 200);
  theGround->RndGroundScaleStepUp();
  theGround->RndGround(100, 100);
  theGround->RndGroundScaleStepUp();
  theGround->RndGround(250, 60);
  theGround->RndGroundScaleStepUp();
  theGround->RndGround(5, 5);
  theGround->RndGroundScaleStepUp();
  theGround->RndGroundTresh(-1);
  theGround->RndGround(3, 10);
  //theGround->RndGround(7.1, 4);

  printKeyList();
  theGround->RndGroundCalcVertNorm();
    
  theCamera->setGround(theGround);

  theCar = new Car(theGround, theCamera);
  
  theGround->setCar(theCar);
    
  //theCamera->setProjection(-.5/4,-.5/4,.2,480);
  theCamera->setProjection(-.5/4,-.5/4,.2,1480);
  //theCamera->setProjection(-.5,-.5,.8,480);
  

  
  glutDisplayFunc(display);
  glutMouseFunc(mousepress);
  glutMotionFunc(mousemotion);
  glutKeyboardFunc(keyboardpress);
  glutTimerFunc(animstep, animate, 0);
  cout << "GO!" << endl;
  glutMainLoop();
  return EXIT_SUCCESS;
}

