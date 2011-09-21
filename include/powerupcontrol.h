#ifndef POWERUPCONTROL
#define POWERUPCONTROL

#include "powerup.h"

#define poweruplen 30

class Graphics; class Net; class Out; class Server;

class Powerupcontrol {
  private:
    Out* out;
    Powerup powerup[poweruplen];

  public:
    Powerupcontrol();
    ~Powerupcontrol();

    void init(Out&, Graphics&);

    int make(float, float, float, PowerupType);
    bool make(int, float, float, float, PowerupType);

    void clear();

    Powerup* getPowerups();

    void go(double, Net&, Server&);

    void draw();

    void local(double);
};

#endif
