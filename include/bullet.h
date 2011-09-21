#ifndef BULLET
#define BULLET

class Graphics; class Net; class Out; class Server;

class Bullet {
  private:
    Out* out;
    Graphics* graphics;
    int dirY, owner;
    float x, y, speedY, width, height;
    bool active, dead;

  public:
    Bullet();

    void init(Out&, Graphics&);

    bool getActive();
    void setActive(bool);

    bool getDead();
    void setDead(bool); // want to get rid of bullet (deactivating may leave it uncleared)

         //      x,    y, dirY, speedY, owner (0-99 players, 100+ baddies)
    void set(float, float, int, float, int);

    void setX(float);
    void setY(float);

    float getX();
    float getY();
    float getWidth();
    float getHeight();
    int getOwner();
    int getDirY();
    float getSpeedY();

    void move(double);

    void go(int, double, Net&, Server&);

    void clear();
    void draw();
};

#endif
