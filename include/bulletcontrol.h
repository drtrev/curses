#ifndef BULLETCONTROL
#define BULLETCONTROL

#include "bullet.h"

#define bulletlen 50

class Graphics; class Net; class Out; class Server;

class Bulletcontrol {
  private:
    Out* out;
    Bullet bullet[bulletlen];

  public:
    Bulletcontrol();
    ~Bulletcontrol();

    void init(Out&, Graphics&);
    
          //      x,    y, dirY, speedY, owner (0-99 players, 100+ baddies)
    bool make(float, float, int, float, int, Net&, Server&);

    Bullet* getBullets();

    void clear();

    void go(double, Net&, Server&);

    void draw();

    void local(double);
};

#endif
