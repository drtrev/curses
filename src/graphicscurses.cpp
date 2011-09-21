#include "graphicscurses.h"
#include "outverbose.h"

bool GraphicsCurses::init(Outverbose &o, WindowInfo w, const char* font, int fontsize)
// font and fontsize not used by graphicscurses
{
  initShared(o);

  // window info not used - just use current terminal

  return true; // success
}

void GraphicsCurses::kill()
{
}

void GraphicsCurses::drawStart()
{
}

void GraphicsCurses::drawBadShip(GraphicsInfo g)
  // set visible to false to clear (i.e. when using curses)
{
  g.texture = 'X';
  if (!g.visible) g.texture = ' ';

  out->addCh((int) (g.y - g.height / 4.0), (int) (g.x - g.width / 4.0), g.texture);
  out->addCh((int) (g.y + g.height / 4.0), (int) (g.x - g.width / 4.0), g.texture);
  out->addCh((int) (g.y + g.height / 4.0), (int) (g.x + g.width / 4.0), g.texture);
  out->addCh((int) (g.y - g.height / 4.0), (int) (g.x + g.width / 4.0), g.texture);
}

void GraphicsCurses::drawBadParticle(GraphicsInfo g)
  // set visible to false to clear (i.e. when using curses)
{
  char c = '0';
  if (!g.visible) c = ' ';

  out->addCh((int) g.y, (int) g.x, c);
}

void GraphicsCurses::drawDebris(GraphicsInfo g)
{
  if (!g.visible) g.texture = ' ';

  out->addCh((int) g.y, (int) g.x, g.texture);
}

void GraphicsCurses::drawMap(std::vector < std::vector < std::vector <double> > > &coords, double zoom, double offsetX, double offsetY)
{
}

void GraphicsCurses::drawPicture(GraphicsInfo g)
{
}

void GraphicsCurses::drawPlayerShip(GraphicsInfo g)
{
  if (!g.visible) {
    out->addCh((int) (g.y - g.height / 4.0), (int) g.x, ' ');
    out->addCh((int) (g.y + g.height / 4.0), (int) g.x, ' ');
    out->addCh((int) (g.y + g.height / 4.0), (int) (g.x - 1), ' ');
    out->addCh((int) (g.y + g.height / 4.0), (int) (g.x + 1), ' ');
  }else{
    out->addCh((int) (g.y - g.height / 4.0), (int) g.x, '^');
    out->addCh((int) (g.y + g.height / 4.0), (int) g.x, 'X');
    out->addCh((int) (g.y + g.height / 4.0), (int) (g.x - 1), '^');
    out->addCh((int) (g.y + g.height / 4.0), (int) (g.x + 1), '^');
  }
}

void GraphicsCurses::drawBullet(GraphicsInfo g)
{
  if (!g.visible) g.texture = ' ';
  else g.texture = '|';

  out->addCh((int) g.y, (int) g.x, g.texture);
}

void GraphicsCurses::drawPowerup(GraphicsInfo g)
{
  if (!g.visible) g.texture = ' ';

  out->addCh((int) g.y, (int) g.x, g.texture);
}

void GraphicsCurses::drawText(GraphicsInfo g)
{
  /*if (!g.visible) {
    for (int i = 0; i < (int) g.text.size(); i++) g.text[i] = ' ';
  }

  // should specify coords here
  out->add(g.text);*/
}

void GraphicsCurses::drawStop()
{
}

void GraphicsCurses::refresh()
{
  // done in clientcontrol and servercontrol
  //out->refreshScreen();
}

float GraphicsCurses::getWidth()
{
  return out->getWidth();
}

