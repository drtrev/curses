#ifndef GRAPHICS
#define GRAPHICS

#include <vector>
#include "windowgen.h"

struct Color {
  float red;
  float green;
  float blue;
  float alpha;
};

struct GraphicsInfo {
  float x;
  float y;
  float z;
  float width;
  float height;
  float depth;
  float angleX;
  float angleY;
  float angleZ;
  float pivotX;
  float pivotY;
  float pivotZ;
  float scaleX;
  float scaleY;
  float scaleZ;

  Color color;

  int texture;
  std::string text;
  bool visible;
};

class Outverbose;

class Graphics {
  protected:
    Outverbose* out;
    WindowGen* window;

  public:
    virtual ~Graphics();

    void initShared(Outverbose&);
    virtual bool init(Outverbose&, WindowInfo, const char*, int) = 0;
    virtual void kill() = 0;

    Color makeColor(float, float, float, float);
    GraphicsInfo makeInfo(float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, Color, int, std::string, bool);
    GraphicsInfo defaultInfo();
    WindowInfo makeWindowInfo(int, int, int, int, bool, bool, int, int, bool, std::string);

    virtual void drawStart() = 0;
    virtual void drawBadShip(GraphicsInfo) = 0;
    virtual void drawBadParticle(GraphicsInfo) = 0;
    virtual void drawBullet(GraphicsInfo) = 0;
    virtual void drawDebris(GraphicsInfo) = 0;
    virtual void drawMap(std::vector< std::vector < std::vector<double> > >&, double, double, double) = 0;
    virtual void drawPicture(GraphicsInfo) = 0;
    virtual void drawPlayerShip(GraphicsInfo) = 0;
    virtual void drawPowerup(GraphicsInfo) = 0;
    virtual void drawText(GraphicsInfo) = 0;
    virtual void drawStop() = 0;

    virtual void refresh() = 0;

    virtual float getWidth() = 0;
};

#endif
