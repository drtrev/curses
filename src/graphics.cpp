#include "graphics.h"
#include "outverbose.h"
#include "windowgenglut.h"

Graphics::~Graphics()
{
}

void Graphics::initShared(Outverbose &o)
{
  out = &o;
}

Color Graphics::makeColor(float red, float green, float blue, float alpha)
{
  Color color = { red, green, blue, alpha };
  return color;
}

GraphicsInfo Graphics::makeInfo(float x, float y, float z, float width, float height, float depth, float angleX, float angleY, float angleZ, float pivotX, float pivotY, float pivotZ, float scaleX, float scaleY, float scaleZ, Color color, int texture, std::string text, bool visible)
{
  GraphicsInfo info = { x, y, z, width, height, depth, angleX, angleY, angleZ, pivotX, pivotY, pivotZ, scaleX, scaleY, scaleZ, color, texture, text, visible };
  return info;
}

GraphicsInfo Graphics::defaultInfo()
{
  GraphicsInfo info = { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, makeColor(1, 1, 1, 1), 0, "", true };
  return info;
}

WindowInfo Graphics::makeWindowInfo(int x, int y, int width, int height, bool doubleBuffer, bool depthBuffer, int refreshRate, int colorDepth, bool fullscreen, std::string title)
{
  return window->makeWindowInfo(x, y, width, height, doubleBuffer, depthBuffer, refreshRate, colorDepth, fullscreen, title);
}

