#include "powerupcontrol.h"
#include "out.h"

Powerupcontrol::Powerupcontrol()
{
}

Powerupcontrol::~Powerupcontrol()
{
}

void Powerupcontrol::init(Out &o, Graphics &g)
{
  out = &o;

  for (int i = 0; i < poweruplen; i++) {
    powerup[i].init(*out, g);
  }
}

int Powerupcontrol::make(float x, float y, float speedY, PowerupType pt)
  // returns ID of powerup or -1 if we've ran out of space!
  // this is for server to call
{
  for (int i = 0; i < poweruplen; i++) {
    if (!powerup[i].getActive()) {
      powerup[i].setActive(true);
      powerup[i].set(x, y, speedY, pt);
      return i;
    }
  }

  return -1;
}

void Powerupcontrol::clear()
{
  for (int i = 0; i < poweruplen; i++) {
    if (powerup[i].getActive()) powerup[i].clear();
  }
}

Powerup* Powerupcontrol::getPowerups()
{
  return powerup;
}

void Powerupcontrol::go(double sync, Net &net, Server &server)
{
  for (int i = 0; i < poweruplen; i++) {
    if (powerup[i].getActive()) powerup[i].go(i, sync, net, server);
  }
}

void Powerupcontrol::draw()
{
  for (int i = 0; i < poweruplen; i++) {
    if (powerup[i].getActive()) powerup[i].draw();
  }
}

void Powerupcontrol::local(double sync)
{
  for (int i = 0; i < poweruplen; i++) {
    if (powerup[i].getActive()) powerup[i].move(sync);
  }
}

