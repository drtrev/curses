#include "powerup.h"
#include "graphics.h"
#include "network/net.h"
#include "network/network.h"
#include "out.h"

Powerup::Powerup()
{
  x = 0, y = 0;
  width = 1, height = 1;
  speedY = 5;
  active = false;
  dead = false; // kill, then deactivate after clear
  icon = 'S';
}

void Powerup::init(Out &o, Graphics &g)
{
  out = &o;
  graphics = &g;
}

bool Powerup::getActive()
{
  return active;
}

void Powerup::setActive(bool a)
{
  active = a;
}

bool Powerup::getDead()
{
  return dead;
}

void Powerup::setDead(bool b)
{
  dead = b;
}

float Powerup::getX()
{
  return x;
}

float Powerup::getY()
{
  return y;
}

float Powerup::getWidth()
{
  return width;
}

float Powerup::getHeight()
{
  return height;
}

void Powerup::set(float sx, float sy, float sspeedY, PowerupType stype)
{
  x = sx, y = sy, speedY = sspeedY;
  setType(stype);
}

PowerupType Powerup::getType()
{
  return type;
}

void Powerup::setType(PowerupType pt)
{
  type = pt;

  switch (type) {
    case POWERUP_SPEED:
      icon = 'S';
      break;
    case POWERUP_BULLET:
      icon = 'B';
      break;
  }
}

void Powerup::clear()
{
  graphics->drawPowerup(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, graphics->makeColor(0, 1, 0, 1), icon, "", false));
  //out->addCh((int) y, (int) x, ' ');
}

void Powerup::move(double sync)
{
  y += speedY * sync;
}

void Powerup::draw()
{
  graphics->drawPowerup(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, graphics->makeColor(0, 1, 0, 1), icon, "", true));
  //out->addCh((int) y, (int) x, icon);
}

void Powerup::go(int id, double sync, Net &net, Server &server)
{
  move(sync);

  if (y < 0 || y > out->getHeight() || dead) {
    if (!dead) {
      // if dead then player checkCollision will send out which player collected it
      Unit unit;
      unit.powerup.flag = UNIT_POWERUP;
      unit.powerup.id = id;
      unit.powerup.x = 0;
      unit.powerup.y = 0;
      unit.powerup.z = 0;
      unit.powerup.speedY = 0;
      unit.powerup.type = 0;
      unit.powerup.collected = -2; // deactivated (out of screen)

      net.addUnitAll(unit, server, -1);
    }

    dead = false;
    active = false;
  }//else{
    //draw();
  //}
}

