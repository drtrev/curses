#ifndef POWERUP
#define POWERUP

enum PowerupType { POWERUP_SPEED, POWERUP_BULLET };

class Graphics; class Net; class Out; class Server;

class Powerup {
  private:
    Out* out;
    Graphics* graphics;
    float x, y, width, height, speedY;
    bool active, dead;
    char icon;
    PowerupType type;

  public:
    Powerup();

    void init(Out&, Graphics&);

    bool getActive();
    void setActive(bool);

    bool getDead();
    void setDead(bool);

    float getX();
    float getY();
    float getWidth();
    float getHeight();

    void set(float, float, float, PowerupType);

    PowerupType getType();
    void setType(PowerupType);

    void clear();

    void move(double);
    void draw();

    void go(int, double, Net&, Server&);

};

#endif
