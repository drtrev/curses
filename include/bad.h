#ifndef BAD
#define BAD

#include "debris.h"
//#include "graphics.h"
#include "powerup.h"
#include <sys/time.h>

#define PI 3.14159265

enum BadMode { BAD_FORMATION, BAD_KAMIKAZEE, BAD_KAMIKAZEE_TO_FORMATION, BAD_HOME };
enum BadType { BAD_SHIP, BAD_PARTICLE };

class Bulletcontrol; class Graphics; class Net; class Outverbose; class Powerupcontrol; class Server; class Timer;

class Bad {
  private:
    Outverbose* out;
    Graphics* graphics;
    float x, y, width, height;
    float angle, speed, turnRate;
    float offsetX, offsetY; // offset from formation
    bool active;
    int type;
    timeval lastFire, fireDelay, lastModeChange, changeMode;
    BadMode mode;
    int health;
    bool dying;
    bool holdFire, holdFormation;
    bool powerup;
    PowerupType powerupType;

    Debris debris[4];

    float getAngle(float, float, float, float);
    void kamikazee(float, float, float, double);
    void home(float, float, double, Timer&);
    void move(float, float, float, float, double, Timer&);
    void fire(int, Bulletcontrol&, Timer&, Net&, Server&);
    void checkCollision(int, Bulletcontrol&, Powerupcontrol&, Net&, Server&);

  public:
    Bad();

    void respawn();

    void init(Outverbose&, Graphics&);

    bool getActive();
    void setActive(bool);

    int getType();
    void setType(BadType);

    void setLastFire(timeval);

    bool getDying();

    BadMode getMode();

    float getOffsetX();
    float getOffsetY();
    void setOffsetX(float);
    void setOffsetY(float);

    float getX();
    void setX(float);
    float getY();
    void setY(float);
    float getWidth();
    float getHeight();
    bool getPowerup();
    void setPowerup(bool);
    void setPowerupType(PowerupType);

    void kill();
    //void checkDead(); // TODO remove
    void dropPowerup(Powerupcontrol&, Net&, Server&);

    void backToFormation();
    void setHoldFormation(bool);
    void setHoldFire(bool);

    void moveDebris(double);

    void go(int, float, float, float, float, double, Bulletcontrol&, Powerupcontrol&, Timer&, Net&, Server&);

    void clear();
    void draw();

};

#endif
