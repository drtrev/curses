#include "debris.h"
#include <cmath>
#include "graphics.h"
#include "out.h"

Debris::Debris()
{
  active = false;
  x = 0, y = 0, speedX = 0, speedY = 0, minSpeed = 5;
  //friction = 8;
  friction = 800; // now using sync with accel
  accelX = 0, accelY = 0;
  width = 1, height = 1;
  icon = 'D';
  color.red = 1, color.green = 1, color.blue = 1, color.alpha = 1;
}

void Debris::init(Out &o, Graphics &g)
{
  out = &o;
  graphics = &g;
}

bool Debris::getActive()
{
  return active;
}

void Debris::setActive(bool a)
{
  active = a;
}

void Debris::set(float sx, float sy, float sspeedX, float sspeedY, float saccelX, float saccelY, Color scolor, char sicon)
{
  x = sx, y = sy, speedX = sspeedX, speedY = sspeedY, accelX = saccelX, accelY = saccelY, color = scolor, icon = sicon;
}

void Debris::move(double sync)
{
  // create a dead zone, because the slow movement as we stop can cause a jump into a new text coord
  //if (accelX == -friction * speedX && fabs(speedX) < minSpeed) speedX = 0;
  //if (accelY == -friction * speedY && fabs(speedY) < minSpeed) speedY = 0;
  // now accel uses sync
  if (fabs(speedX) < minSpeed) speedX = 0;
  if (fabs(speedY) < minSpeed) speedY = 0;

  // s = ut + 0.5 * att
  // s is displacement. s equals ut plus a half a t-squared
  // u is initial velocity, t is time, using 0 as time started accelerating

  x += speedX * sync + 0.5 * accelX * sync * sync;
  y += speedY * sync + 0.5 * accelY * sync * sync;

  // update velocity
  // v = u + at
  speedX = speedX + accelX * sync;
  speedY = speedY + accelY * sync;

  // friction
  // this is a percentage of speed, i.e. the faster you move the more air resistance
  // this should really take into account time, see http://en.wikipedia.org/wiki/Drag_(physics)
  // This works here with sync, but friction is assumed constant over that time period,
  // so if it is running significantly slower then it will stop noticably quicker
  // Think integral approximations: need small time intervals to be more precise.
  accelX = -friction * speedX * sync;
  accelY = -friction * speedY * sync;

  // check if finished
  if (fabs(accelX) < 0.1 && fabs(accelY) < 0.1 && fabs(speedX) < 0.01 && fabs(speedY) < 0.01) active = false;
}

void Debris::clear()
{
  graphics->drawDebris(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, color, icon, "", false));
  //out->addCh((int) y, (int) x, ' ');
}

void Debris::draw()
{
  graphics->drawDebris(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, color, icon, "", true));
  //out->addCh((int) y, (int) x, icon);
}
