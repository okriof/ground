// SnowRacer
// Kristoffer 2009-04

// ground.h
// Marken

#ifndef GROUND_H
#define GROUND_H
#include <GL/glut.h>
// DRAWNORMALS och DRAWWIRE från demodefines.h

// villkor för byte av terrängupplösning
#define MIPSWITCHCONST 10000
// parametrar för gaussiskt LP-filter (för mipmap-generering)
#define LPFILTRADIUS 2
#define LPFILTSIGMA .8
// hårdhetsfaktor hos marken (hur snabbt den hårdnar när den trycks ihop)
#define HARDENINGFACTOR 2


class Camera;
class Car;

class Ground {
public:
            Ground(unsigned int power2initSize, GLfloat span, Camera* theCamera);      
            ~Ground();
            
            // markskaparfunktioner, anropa RndGrouncCalcVertNorm sist
    void    RndGround(GLfloat maxdev, int smoothloops);
    void    RndGroundTresh(GLfloat level);
    void    RndGroundScaleStepUp();
    void    RndGroundCalcVertNorm();
    
    void    setCar(Car* car) {theCar = car; };
    
    void    Draw(); // rita marken
    GLfloat Height(GLfloat position[]); // ger markhöjden vid angiven punkt
    void    Normal(GLfloat x, GLfloat z, GLfloat* normalv); // ger marknormalen vid punkten
      
    void    setHeight(GLfloat x, GLfloat z, GLfloat height, GLfloat radius); // används för att trycka ner marken 
      
private:    
    void    recalcVertNorm(unsigned int r, unsigned int k, unsigned int radius);  // beräkna om mipmap samt normaler i ett område
    
    void    drawRec(int level, unsigned int rC, unsigned int kC, unsigned char subSqNo); //rita (rekursiva delen)
    void    drawFan(int level, unsigned int rC, unsigned int kC, unsigned char subSqNo); //rita en ruta
    
            // falta med roterad kärna (normalt LP- eller deriveringsfilter)
    void    conv(GLfloat f[], unsigned int maxfI, GLfloat kern[], unsigned int maxkernI,
                 GLfloat output[], unsigned int rmin, unsigned int kmin, unsigned int rmax, unsigned int kmax);    
            
            // falta med roterad kärna + decimering
    void    convDS(GLfloat f[], unsigned int maxfI, GLfloat kern[], unsigned int maxkernI,
                   GLfloat output[], unsigned int rmin, unsigned int kmin, unsigned int rmax, unsigned int kmax);    


    void    RndGroundFixEdge(); // plocka ned höjden längs kanterna
            
    GLfloat* heights;    // markhöjden, efter markgeneration klar, även alla mipmap-nivåer
    GLfloat* hardness;   // hårdhet för olika delar av marken
    
    GLfloat* dheightdr;  // derivatabuffertar
    GLfloat* dheightdk;
    
    GLfloat (*vertices)[3]; // markverticies
    GLfloat (*normals)[3];  // marknormaler
    GLfloat span;           // avstånd mellan närmaste två punkterna i marken
    unsigned int rows;      // storlek
    unsigned int cols;
    
    char* neighbourLOD;     // mipmap-nivåer för olika delar
    char* neighbourLOD2;
    
    GLfloat* planeNormals;  // frustum-plan och punkt
    GLfloat* planePointNormalProds;
    
             int  levelCount;         // antalet mipmap-nivåer
    unsigned int* levelIndexOffsets;  // varje nivås index-offset i alla vektorer
    GLfloat*      levelSpan;          // storlek för alla olika mipmap-nivåer

    GLfloat*      lpFilt;   // antivikningsfilterkärna
    GLfloat*      ddrFilt;  // d/dr-filterkärna (för normalberäkning)
    GLfloat*      ddkFilt;  // d/dk-filterkärna

    Camera*        theCamera; // kameran
    Car*           theCar;    // pekare till bilen, används för att hantera utanför-banan
    const GLfloat* camPos;    // kamerans position (för mipmap-nivåbestämning)
};


#endif
