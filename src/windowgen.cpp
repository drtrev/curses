#include "windowgen.h"

#include <iostream>

WindowGen::WindowGen()
{
  wid = -1;
}

WindowGen::~WindowGen()
{
}

WindowInfo WindowGen::makeWindowInfo(int x, int y, int width, int height, bool doubleBuffer, bool depthBuffer, int refreshRate, int colorDepth, bool fullscreen, std::string title)
{
  WindowInfo info = { x, y, width, height, doubleBuffer, depthBuffer, refreshRate, colorDepth, fullscreen, title };
  return info;
}

int WindowGen::getWid()
{
  return wid;
}

int WindowGen::getWidth()
{
  return winfo.width;
}

int WindowGen::getHeight()
{
  return winfo.height;
}

void WindowGen::resize(int w, int h)
{
  winfo.width = w;
  winfo.height = h;
}

void WindowGen::initShared(Outverbose& o, WindowInfo w)
{
  out = &o;
}

