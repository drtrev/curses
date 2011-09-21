#ifndef PLAYER
#define PLAYER

#include "debris.h"
#include "powerup.h"
#include <sys/time.h>

class Badcontrol; class Bulletcontrol; class Net; class Outverbose; class Powerupcontrol; class Server; class Timer;

class Player {
  private:
    Graphics* graphics;
    Outverbose* out;

    Debris debris[4];

    timeval lastFire, deadTime;

    float startX, startY;
    float x, y, accelX, accelY, power, friction, speedX, speedY, width, height;
    float oldAccelX, oldAccelY;
    float minSpeed; // stop jumping into next text coord
    int health, startHealth, lives, startLives, bullets, startBullets, maxBullets;
    float startPower, maxPower, powerIncrement; // for powerups changing power
    bool dying;

    void fire(int, Bulletcontrol&, Timer&, Net&, Server&);
    void collect(PowerupType);

    void moveDebris(double);

  public:
    Player();
    ~Player();

    void init(Outverbose&, Graphics&);

    float getX();
    float getY();

    void setX(float);
    void setY(float);

    bool getDying();
    int getLives();
    timeval getDeadTime();

    void local(double);

    void move(double);
    void clear();
    void draw();
    void checkCollision(int, Bulletcontrol&, Badcontrol&, Powerupcontrol&, timeval, Net&, Server&);

    void kill();

    void respawnLocal();
    void respawn(int, Badcontrol&, Net&, Server&);
    void reset(Badcontrol&);

    void input(int, int, Bulletcontrol&, Timer&, double, Net&, Server&);
};

#endif
