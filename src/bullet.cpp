#include "bullet.h"
#include "graphics.h"
#include "network/net.h"
#include "network/network.h"
#include "out.h"

Bullet::Bullet()
{
  owner = -1;
  x = 0, y = 0;
  width = 1, height = 1;
  dirY = 1, speedY = 0;
  active = false;
  dead = false; // kill, then deactivate after transmitting that it's dead
}

void Bullet::init(Out &o, Graphics &g)
{
  out = &o;
  graphics = &g;
}

bool Bullet::getActive()
{
  return active;
}

void Bullet::setActive(bool b)
{
  active = b;
}

bool Bullet::getDead()
{
  return dead;
}

void Bullet::setDead(bool b)
{
  dead = b;
}

void Bullet::set(float sx, float sy, int sdirY, float sspeedY, int sowner)
{
  x = sx, y = sy, dirY = sdirY, speedY = sspeedY, owner = sowner;
}

void Bullet::setX(float sx)
{
  x = sx;
}

void Bullet::setY(float sy)
{
  y = sy;
}

float Bullet::getX()
{
  return x;
}

float Bullet::getY()
{
  return y;
}

float Bullet::getWidth()
{
  return width;
}

float Bullet::getHeight()
{
  return height;
}

int Bullet::getOwner()
{
  return owner;
}

int Bullet::getDirY()
{
  return dirY;
}

float Bullet::getSpeedY()
{
  return speedY;
}

void Bullet::clear()
{
  graphics->drawBullet(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, graphics->makeColor(1, 1, 1, 1), 0, "", false));
  //out->addCh((int) y, (int) x, ' ');
}

void Bullet::move(double sync)
{
  y += dirY * speedY * sync;
}

void Bullet::draw()
{
  graphics->drawBullet(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, graphics->makeColor(1, 1, 1, 1), 0, "", true));
  //out->addCh((int) y, (int) x, '|');
}

void Bullet::go(int id, double sync, Net &net, Server &server)
  // call after clear
{
  move(sync);

  if (y < 0 || y > out->getHeight() || dead) {
    // transmit here cos this covers dead and out of screen
    Unit unit;
    unit.bullet.flag = UNIT_BULLET;
    unit.bullet.id = id;
    unit.bullet.active = 0;
    unit.bullet.x = 0;
    unit.bullet.y = 0;
    unit.bullet.z = 0;
    unit.bullet.dirY = 0;
    unit.bullet.speedY = 0;
    unit.bullet.owner = 0;

    net.addUnitAll(unit, server, -1);

    dead = false;
    active = false;
  }//else{
    //draw();
  //}

}

