#ifndef GRAPHICS_OPENGL
#define GRAPHICS_OPENGL

#include "graphics.h"

#define OGLFT_NO_SOLID // don't use GLE (OpenGL Tubing and Extrusion library)
#define OGLFT_NO_QT // don't use QT (which is for glyphs)
#include <OGLFT.h>

#include <vector>

class Outverbose;

class GraphicsOpenGL : public Graphics {
  private:
    struct Frustum {
      float top;
      float right;
      float near;
      float far;
    } frustum;

    // store the square border for full screen pictures
    struct Border {
      float top;
      float right;
      float bottom;
      float left;
    } border;

    // see demos and http://oglft.sourceforge.net/ for a review of FPS rates and appearance for each type
    // NOTE if change this here, then need to change font initialisation in init()

    //OGLFT::Translucent* face; // this one looks best, but is slow
    OGLFT::TranslucentTexture* face; // this is faster

    void flip();

  public:
    GraphicsOpenGL();

    //bool init(Outverbose&, WindowInfo);
    bool init(Outverbose&, WindowInfo, const char* font, int fontsize);
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
