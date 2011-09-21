#ifndef PICTURE
#define PICTURE

#include <string>
#include "textures.h"
#include <vector>

class Graphics; class Outverbose; class Textures;

class Picture {
  private:
    int width;
    int height;
    float x, y, z, targetX, targetY, targetZ, speed;
    float angleX, angleY, angleZ, targetAngleY, speedRot;
    float pivotX, pivotY, pivotZ;
    float scaleX, scaleY, scaleZ;
    float offsetX, offsetY, offsetZ;
    bool increasingOffset, increasingZoom;
    int direction;
    int texId;
    int loaded;

    bool active, requested;

    Graphics* graphics;
    Outverbose* out;
    Textures* textures;

  public:
    Picture();

    void init(Outverbose&, Graphics&, Textures&);
    void load(std::string);
    void incrementLoaded();
    void setActive(bool);
    bool getActive();
    void setRequested(bool);
    bool getRequested();
    int getWidth();
    int getHeight();
    float getX();
    float getY();
    float getZ();
    float getAngleY();
    void setX(float);
    void setY(float);
    void setZ(float);
    void setAngleY(float);
    void setSpeedRot(float);

    void setTargetX(float);
    void setTargetZ(float);
    void setTargetAngleY(float);

    void setDirection(int);
    void loopAngles();

    void move(double);

    void draw(int);

};

#endif
