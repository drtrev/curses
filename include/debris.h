#ifndef DEBRIS
#define DEBRIS

#include "graphics.h"

class Out;

class Debris {
  private:
    Graphics* graphics;
    Out* out;

    Color color;

    float x, y, friction, speedX, speedY, accelX, accelY, width, height;
    float minSpeed; // stop jumping into next text coord

    bool active;

    char icon;

  public:
    Debris();

    void init(Out&, Graphics&);

    bool getActive();
    void setActive(bool);
    void set(float, float, float, float, float, float, Color, char);

    void move(double);

    void clear();
    void draw();
};

#endif
