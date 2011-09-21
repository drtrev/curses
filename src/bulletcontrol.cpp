#include "bulletcontrol.h"
#include "network/net.h"
#include "network/network.h"

Bulletcontrol::Bulletcontrol()
{
}

Bulletcontrol::~Bulletcontrol()
{
}

void Bulletcontrol::init(Out &o, Graphics &g)
{
  out = &o;

  for (int i = 0; i < bulletlen; i++) {
    bullet[i].init(*out, g);
  }
}

bool Bulletcontrol::make(float x, float y, int dirY, float speedY, int owner, Net &net, Server &server)
{
  Unit unit;
  unit.flag = UNIT_BULLET;
  unit.bullet.active = 1;
  unit.bullet.x = x;
  unit.bullet.y = y;
  unit.bullet.z = 0;
  unit.bullet.dirY = dirY;
  unit.bullet.speedY = speedY;
  unit.bullet.owner = owner;

  for (int i = 0; i < bulletlen; i++) {
    if (!bullet[i].getActive()) {
      bullet[i].setActive(true);
      bullet[i].set(x, y, dirY, speedY, owner);

      unit.bullet.id = i;
      net.addUnitAll(unit, server, -1);

      return true;
    }
  }

  return false;
}

Bullet* Bulletcontrol::getBullets()
{
  return bullet;
}

void Bulletcontrol::clear()
{
  for (int i = 0; i < bulletlen; i++) {
    if (bullet[i].getActive()) bullet[i].clear();
  }
}

void Bulletcontrol::go(double sync, Net &net, Server &server)
  // call after clear
{
  for (int i = 0; i < bulletlen; i++) {
    if (bullet[i].getActive()) bullet[i].go(i, sync, net, server);
  }
}

void Bulletcontrol::draw()
{
  for (int i = 0; i < bulletlen; i++) {
    if (bullet[i].getActive()) bullet[i].draw();
  }
}

void Bulletcontrol::local(double sync)
{
  for (int i = 0; i < bulletlen; i++) {
    if (bullet[i].getActive()) bullet[i].move(sync);
  }
}

