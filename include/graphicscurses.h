#ifndef GRAPHICS_CURSES
#define GRAPHICS_CURSES

#include "graphics.h"
#include <vector>

class Outverbose;

class GraphicsCurses : public Graphics {
  private:

  public:
    bool init(Outverbose&, WindowInfo, const char*, int);
    void kill();

    void drawStart();
    void drawBadShip(GraphicsInfo);
    void drawBadParticle(GraphicsInfo);
    void drawBullet(GraphicsInfo);
    void drawDebris(GraphicsInfo);
    void drawMap(std::vector < std::vector < std::vector <double> > >&, double, double, double);
    void drawPicture(GraphicsInfo);
    void drawPlayerShip(GraphicsInfo);
    void drawPowerup(GraphicsInfo);
    void drawText(GraphicsInfo);
    void drawStop();

    void refresh();

    float getWidth();
};

#endif
