#include "badcontrol.h"
#include <cstdlib>
#include "network/net.h"
#include "network/network.h"
#include "outverbose.h"
#include "timer.h"

Badcontrol::Badcontrol()
{
  formationX = 1, formationY = 1;
  speedX = 10;
  speedInc = 0;

  spawning = false;
  spawnOffsetX = 0, spawnOffsetY = 0;

  moveDown = true;

  lastSpawn.tv_sec = 0;
  lastSpawn.tv_usec = 0;

  spawnDelay.tv_sec = std::rand() % 10 + 3;
  spawnDelay.tv_usec = std::rand() % 100 * 10000;

  attackWave = 0;
}

Badcontrol::~Badcontrol()
{

}

void Badcontrol::init(Outverbose &o, Graphics &g)
{
  out = &o;

  for (int i = 0; i < badlen; i++) {
    bad[i].init(*out, g);
  }
}

void Badcontrol::clearPowerups()
{
  for (int i = 0; i < badlen; i++) {
    bad[i].setPowerup(false);
  }
}

void Badcontrol::makePowerups(int pows, int bads, PowerupType type)
  // input number of powerups and baddies
  // assumes powerups were cleared first with clearPowerups
  // also only one powerup per bad so pows must be less than or equal to
  // number of baddies without powerups
{
  int ratio = bads/pows;

  // try every ratio
  for (int i = 0; i < badlen && i < bads && pows > 0; i += ratio) {
    if (bad[i].getActive()) {
      if (!bad[i].getPowerup()) {
        bad[i].setPowerup(true);
        bad[i].setPowerupType(type);
        pows--;
      }else{
        i -= ratio;
        i++; // next loop try next baddie
      }
    }
  }

  // try to assign any left over
  if (bads < badlen + 1) {
    for (int i = bads - 1; i >= 0 && pows > 0; i--) {
      if (bad[i].getActive() && !bad[i].getPowerup()) {
        bad[i].setPowerup(true);
        bad[i].setPowerupType(type);
        pows--;
      }
    }
  }
}

void Badcontrol::makeBad(int i, float sx, float sy, float ox, float oy, BadType type, timeval currentTime)
{
  bad[i].respawn();
  bad[i].setActive(true);
  bad[i].setType(type);
  bad[i].setOffsetX(ox);
  bad[i].setOffsetY(oy);
  bad[i].setX(sx);
  bad[i].setY(sy);
  bad[i].setLastFire(currentTime);
}

void Badcontrol::makeAttack(int wave, timeval currentTime)
{
  clearPowerups();
  spawning = false;
  lastSpawn = currentTime;
  moveDown = true;
  speedInc = 2;

  // make all inactive for client's sake
  for (int i = 0; i < badlen; i++) {
    bad[i].setActive(false);
  }

#define attacklen 2

  switch (wave) {
    case 0:
      formationX = 1, formationY = 1;
      speedX = 10;

      for (int i = 0; i < badlen && i < 10; i++) {
        makeBad(i, i*3, -5, i%5*3, i/5*3, BAD_SHIP, currentTime);
      }
      makePowerups(5, 10, POWERUP_SPEED);
      makePowerups(2, 10, POWERUP_BULLET);
      break;
    case 1:
      formationX = 1, formationY = 2;
      speedX = 10;

      for (int i = 0; i < badlen && i < 19; i++) {
        makeBad(i, i*3, -5, i%10*6+i/10*3, i/10*3, BAD_SHIP, currentTime);
      }
      makePowerups(3, 15, POWERUP_SPEED);
      makePowerups(3, 15, POWERUP_BULLET);
      break;
    case 2:
      formationX = 1, formationY = 1;
      speedX = 10;
      speedInc = 4;
      spawning = true;
      spawnOffsetX = 6.5;
      spawnOffsetY = 1.5;

      for (int i = 0; i < badlen && i < 7; i++) {
        makeBad(i, i*3, -5, i+3, 0, BAD_PARTICLE, currentTime);
      }
      for (int i = 7; i < badlen && i < 16; i++) {
        makeBad(i, i*3, -5, i-7+2, 1, BAD_PARTICLE, currentTime);
      }
      for (int i = 16; i < badlen && i < 25; i++) {
        makeBad(i, i*3, -5, i-16+2, 2, BAD_PARTICLE, currentTime);
      }
      for (int i = 25; i < badlen && i < 34; i++) {
        makeBad(i, i*3, -5, i-25+2, 3, BAD_PARTICLE, currentTime);
      }
      for (int i = 34; i < badlen && i < 41; i++) {
        makeBad(i, i*3, -5, i-34+3, 4, BAD_PARTICLE, currentTime);
      }
      makePowerups(5, 41, POWERUP_SPEED);
      makePowerups(5, 41, POWERUP_BULLET);
      break;
    default:
      *out << VERBOSE_LOUD << "Error, attack wave not found: " << wave << '\n';
  }

}

void Badcontrol::clear()
{
  for (int i = 0; i < badlen; i++) {
    if (bad[i].getActive()) bad[i].clear();
  }
}

/*void Badcontrol::checkDead()
{
  for (int i = 0; i < badlen; i++) {
    if (bad[i].getActive()) bad[i].checkDead();
  }
}*/ // TODO remove

Bad* Badcontrol::getBaddies()
{
  return bad;
}

void Badcontrol::sendAllBackToFormation()
{
  for (int i = 0; i < badlen; i++) {
    if (bad[i].getActive() && !bad[i].getDying()) bad[i].backToFormation();
  }
}

void Badcontrol::holdFormation(bool b)
{
  for (int i = 0; i < badlen; i++) {
    bad[i].setHoldFormation(b);
  }
}

void Badcontrol::holdFire(bool b)
{
  for (int i = 0; i < badlen; i++) {
    bad[i].setHoldFire(b);
  }
}

void Badcontrol::spawn(Timer &timer, Net &net, Server &server)
{
  timeval elapsed = timer.elapsed(lastSpawn);

  if (elapsed.tv_sec > spawnDelay.tv_sec
    || (elapsed.tv_sec == spawnDelay.tv_sec && elapsed.tv_usec > spawnDelay.tv_usec))
  {
    lastSpawn = timer.getCurrent();

    spawnDelay.tv_sec = std::rand() % 10 + 1;
    spawnDelay.tv_usec = std::rand() % 100 * 10000;

    Unit unit;

    for (int i = 0; i < badlen; i++) {
      if (!bad[i].getActive()) {
        makeBad(i, spawnOffsetX + formationX, spawnOffsetY + formationY, spawnOffsetX, spawnOffsetY, BAD_SHIP, timer.getCurrent());

        unit.position.flag = UNIT_POSITION;
        unit.position.id = i + ID_BAD_MIN;
        unit.position.x = bad[i].getX();
        unit.position.y = bad[i].getY();
        unit.position.z = 0;
        net.addUnitAll(unit, server, -1);

        unit.bad.flag = UNIT_BAD;
        unit.bad.id = i;
        unit.bad.type = bad[i].getType();
        unit.bad.status = 1;
        net.addUnitAll(unit, server, -1);
        
        if (std::rand() % 2 == 0) bad[i].setPowerup(true);
        else bad[i].setPowerup(false);

        if (std::rand() % 2 == 0) bad[i].setPowerupType(POWERUP_SPEED);
        else bad[i].setPowerupType(POWERUP_BULLET);

        break;
      }
    }
  }
}

void Badcontrol::go(float playerX, float playerY, double sync, Bulletcontrol &bulletcontrol, Powerupcontrol &powerupcontrol, Timer &timer, bool &gameover, Net &net, Server &server)
{
  // s = ut + 0.5 * att
  // a = 0
  formationX += speedX * sync;

  // check if any baddie has hit the edge of the screen, using their place in formation
  // If they have, change formation direction
  for (int i = 0; i < badlen; i++) {
    if (bad[i].getActive() && !bad[i].getDying()) {
      if ((formationX + bad[i].getOffsetX() + bad[i].getWidth() / 2.0 > out->getWidth() && speedX > 0.0)
          || (formationX + bad[i].getOffsetX() - bad[i].getWidth() / 2.0 < 0 && speedX < 0.0))
      {
        speedX = -speedX;
        if (moveDown) {
          formationY++;
          speedX = (speedX < 0) ? speedX - speedInc : speedX + speedInc;
          for (int i = 0; i < badlen; i++) {
            if (bad[i].getActive() && !bad[i].getDying()) {
              if (formationY + bad[i].getOffsetY() + bad[i].getHeight() / 2.0 > out->getHeight()) {
                gameover = true;
              }
            }
          }
        }
        break;
      }
    }
  }

  if (spawning) spawn(timer, net, server);

  int dead = 0;

  for (int i = 0; i < badlen; i++) {
    if (bad[i].getActive()) bad[i].go(i, formationX, formationY, playerX, playerY, sync, bulletcontrol, powerupcontrol, timer, net, server);
    else dead++;
  }

  if (dead == badlen) {
    makeAttack(attackWave, timer.getCurrent());

    Unit unit;
    unit.attack.flag = UNIT_ATTACK;
    unit.attack.wave = attackWave;

    net.addUnitAll(unit, server, -1);

    attackWave++;
    if (attackWave > attacklen) attackWave = 0;
  }
}

void Badcontrol::draw()
{
  for (int i = 0; i < badlen; i++) {
    if (bad[i].getActive()) bad[i].draw();
  }
}

void Badcontrol::local(double sync)
  // stuff that's left up to local calculations (e.g. explosions)
{
  for (int i = 0; i < badlen; i++) {
    if (bad[i].getActive() && bad[i].getDying()) bad[i].moveDebris(sync);
  }
}

