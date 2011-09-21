#ifndef BADCONTROL
#define BADCONTROL

#include "bad.h"

#define badlen 50

class Bulletcontrol; class Graphics; class Net; class Outverbose; class Server; class Timer;

class Badcontrol {
  private:
    Outverbose* out;
    Bad bad[badlen];
    float formationX, formationY, speedX, speedInc;
    int attackWave;
    bool spawning, moveDown;
    float spawnOffsetX, spawnOffsetY;
    timeval lastSpawn, spawnDelay;

    void clearPowerups();
    void makePowerups(int, int, PowerupType);
    void makeBad(int, float, float, float, float, BadType, timeval);

  public:
    Badcontrol();
    ~Badcontrol();

    void init(Outverbose&, Graphics&);

    void clear();
    //void checkDead(); // TODO remove

    Bad* getBaddies();

    void sendAllBackToFormation();
    void holdFormation(bool);
    void holdFire(bool);

    void makeAttack(int, timeval); // can be called by network

    void spawn(Timer&, Net&, Server&);

    void go(float, float, double, Bulletcontrol&, Powerupcontrol&, Timer&, bool&, Net&, Server&);

    void draw();

    void local(double); // for stuff that's done locally
};

#endif
