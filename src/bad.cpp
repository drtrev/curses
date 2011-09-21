#include "bad.h"
#include "bulletcontrol.h"
#include <cstdlib>
#include <cmath>
#include "graphics.h"
#include "network/net.h"
#include "network/network.h"
#include "outverbose.h"
#include "powerupcontrol.h"
#include "timer.h"

Bad::Bad()
{
  width = 2, height = 2;
  type = BAD_SHIP;
  x = width / 2.0, y = height / 2.0, offsetX = 0, offsetY = 0;
  active = false;
  lastFire.tv_sec = 0; // should be set by badcontrol using setLastFire
  lastFire.tv_usec = 0;
  fireDelay.tv_sec = std::rand() % 10 + 1;
  fireDelay.tv_usec = std::rand() % 100 * 10000;
  mode = BAD_KAMIKAZEE_TO_FORMATION;
  angle = PI;
  speed = 40;
  turnRate = 10;
  lastModeChange.tv_sec = 0; // will be set straight away in move with no mode change
  lastModeChange.tv_usec = 0;
  changeMode.tv_sec = std::rand() % 20 + 3;
  changeMode.tv_usec = std::rand() % 100 * 10000;

  health = 1;
  dying = false;
  //dead = false;
  holdFire = false;
  holdFormation = false;

  powerup = true;
  powerupType = POWERUP_SPEED;
}

void Bad::respawn()
{
  mode = BAD_KAMIKAZEE_TO_FORMATION;
  angle = PI;
  health = 1;
  dying = false;
  //dead = false;
  lastModeChange.tv_sec = 0; // will be set straight away in move with no mode change
  lastModeChange.tv_usec = 0;
}

void Bad::init(Outverbose &o, Graphics& g)
{
  out = &o;
  graphics = &g;
  for (int i = 0; i < 4; i++) {
    debris[i].init(*out, *graphics);
  }
}

bool Bad::getActive()
{
  return active;
}

void Bad::setActive(bool b)
{
  active = b;
}

int Bad::getType()
{
  return type;
}

void Bad::setType(BadType t)
{
  type = t;

  switch (type) {
    case BAD_SHIP:
      width = 2, height = 2;
      health = 1;
      break;
    case BAD_PARTICLE:
      width = 1, height = 1;
      health = 3;
      break;
  }
}

void Bad::setLastFire(timeval t)
{
  lastFire = t;
}

bool Bad::getDying()
{
  return dying;
}

BadMode Bad::getMode()
{
  return mode;
}

float Bad::getOffsetX()
{
  return offsetX;
}

float Bad::getOffsetY()
{
  return offsetY;
}

void Bad::setOffsetX(float o)
{
  offsetX = o;
}

void Bad::setOffsetY(float o)
{
  offsetY = o;
}

float Bad::getX()
{
  return x;
}

void Bad::setX(float f)
{
  x = f;
}

float Bad::getY()
{
  return y;
}

void Bad::setY(float f)
{
  y = f;
}

float Bad::getWidth()
{
  return width;
}

float Bad::getHeight()
{
  return height;
}

bool Bad::getPowerup()
{
  return powerup;
}

void Bad::setPowerup(bool p)
{
  powerup = p;
}

void Bad::setPowerupType(PowerupType pt)
{
  powerupType = pt;
}

void Bad::clear()
{
  if (dying) {
    for (int i = 0; i < 4; i++) {
      debris[i].clear();
    }
  }else{
    switch (type) {
      case BAD_SHIP:
        graphics->drawBadShip(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, graphics->makeColor(1, 0, 0, 1), 0, "", false));
        /*out->addCh((int) (y - height / 4.0), (int) (x - width / 4.0), ' ');
        out->addCh((int) (y + height / 4.0), (int) (x - width / 4.0), ' ');
        out->addCh((int) (y + height / 4.0), (int) (x + width / 4.0), ' ');
        out->addCh((int) (y - height / 4.0), (int) (x + width / 4.0), ' ');*/
        break;
      case BAD_PARTICLE:
        graphics->drawBadParticle(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, graphics->makeColor(1, 0, 0, 1), 0, "", false));
        //out->addCh((int) y, (int) x, ' ');
        break;
    }
  }
}

float Bad::getAngle(float x1, float y1, float x2, float y2)
  // get angle between two points
  // zero is negative y, counting round clockwise
{
  float a; // the angle

  float dx = fabs(x2 - x1), dy = fabs(y2 - y1);

  if (x2 > x1 && y2 > y1) {
    // tan a = dx / dy
    a = PI - atan(dx / dy);
  }else if (x2 < x1 && y2 > y1) {
    // tan a = dx / dy
    a = PI + atan(dx / dy);
  }else if (x2 < x1 && y2 < y1) {
    // tan a = dy / dx
    a = PI / 2.0 * 3 + atan(dy / dx);
  }else if (x2 > x1 && y2 < y1) {
    // tan a = dx / dy
    a = atan(dx / dy);
  }else if (x1 == x2 && y2 > y1) {
    a = PI;
  }else if (x1 == x2 && y2 < y1) {
    a = 0;
  }else if (x2 > x1 && y1 == y2) {
    a = PI / 2.0;
  }else if (x2 < x1 && y1 == y2) {
    a = PI / 2.0 * 3;
  }else{
    // x1 == x2 && y1 == y2
    a = 0;
  }

  return a;
}

void Bad::kamikazee(float targetX, float targetY, float turn, double sync)
{
  x += speed * sync * sin(angle);
  y += speed * sync * -cos(angle);

  if (angle < 0) angle += 2 * PI;
  if (angle > 2 * PI) angle -= 2 * PI;

  float targetAngle = getAngle(x, y, targetX, targetY);

  // which way's quicker? turn left or right?
  float diff = angle - targetAngle;

  if ((diff > 0 && diff < PI) || diff < -PI)
    angle -= turn * sync;
  else
    angle += turn * sync;
}

void Bad::home(float targetX, float targetY, double sync, Timer &timer)
// quick and dirty per frame home in on target in a number of steps
{
  static float distX = 0, distY = 0;
  static int count = 0;

  if (count == 0) {
    // initialise
    distX = (targetX - x) / 5.0; // 5 steps
    distY = (targetY - y) / 5.0;
  }

  x += distX;
  y += distY;

  count++;

  // check if finished
  if (count > 4)
  {
    distX = 0, distY = 0;
    angle = PI;
    x = targetX;
    y = targetY;

    mode = BAD_FORMATION;

    changeMode.tv_sec = std::rand() % 20 + 3;
    changeMode.tv_usec = std::rand() % 100 * 10000;

    lastModeChange = timer.getCurrent();
  }
}

void Bad::moveDebris(double sync)
  // note seperate function so can call locally
{
  int deadDebris = 0;
  for (int i = 0; i < 4; i++) {
    if (debris[i].getActive()) debris[i].move(sync);
    else deadDebris++;
  }
  if (deadDebris == 4) active = false;
}

void Bad::move(float formationX, float formationY, float playerX, float playerY, double sync, Timer &timer)
{
  if (dying) {
    moveDebris(sync);
  }else{
    timeval elapsed = timer.elapsed(lastModeChange);

    if (elapsed.tv_sec > changeMode.tv_sec
        || (elapsed.tv_sec == changeMode.tv_sec && elapsed.tv_usec > changeMode.tv_usec)) {

      lastModeChange = timer.getCurrent();

      if (mode == BAD_FORMATION && !holdFormation && type != BAD_PARTICLE) {
        mode = BAD_KAMIKAZEE;

        changeMode.tv_sec = std::rand() % 5 + 1;
        changeMode.tv_usec = std::rand() % 100 * 10000;

      }else if (mode == BAD_KAMIKAZEE) {
        mode = BAD_KAMIKAZEE_TO_FORMATION;
      }
    }

    if (mode == BAD_FORMATION) {
      x = formationX + offsetX;
      y = formationY + offsetY;
    }else if (mode == BAD_KAMIKAZEE) {
      kamikazee(playerX, playerY, turnRate, sync);
    }else if (mode == BAD_KAMIKAZEE_TO_FORMATION) {
      // check for dead zone
      float deadRadius = 1, targetX = formationX + offsetX, targetY = formationY + offsetY;

      if (x > targetX - deadRadius && x < targetX + deadRadius
          && y > targetY - deadRadius && y < targetY + deadRadius)
      {
        mode = BAD_HOME;
      }

      kamikazee(targetX, targetY, turnRate * 2, sync);
    }else if (mode == BAD_HOME) {
      home(formationX + offsetX, formationY + offsetY, sync, timer);
    }
  }
}

void Bad::fire(int id, Bulletcontrol &bulletcontrol, Timer &timer, Net &net, Server &server)
{
  // making fireDelay static would mean it's the same for all instances!!
  timeval elapsed = timer.elapsed(lastFire);

  if (elapsed.tv_sec > fireDelay.tv_sec
    || (elapsed.tv_sec == fireDelay.tv_sec && elapsed.tv_usec > fireDelay.tv_usec))
  {
    lastFire = timer.getCurrent();

    switch (type) {
      case BAD_SHIP:
        fireDelay.tv_sec = std::rand() % 10 + 1;
        fireDelay.tv_usec = std::rand() % 100 * 10000;
        break;
      case BAD_PARTICLE:
        fireDelay.tv_sec = std::rand() % 10 + 7;
        fireDelay.tv_usec = std::rand() % 100 * 10000;
        break;
    }

    if (!holdFire) {
      if (!bulletcontrol.make(x, y + height / 2.0, 1, 30, id + ID_BAD_MIN, net, server)) {
        *out << VERBOSE_LOUD << "Bullet overflow!\n";
      }
    }
  }
}

void Bad::draw()
{
  if (dying) {
    for (int i = 0; i < 4; i++) {
      if (debris[i].getActive()) debris[i].draw();
    }
  }else{
    switch (type) {
      case BAD_SHIP:
        graphics->drawBadShip(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, graphics->makeColor(1, 0, 0, 1), 0, "", true));
        /*out->addCh((int) (y - height / 4.0), (int) (x - width / 4.0), 'X');
        out->addCh((int) (y + height / 4.0), (int) (x - width / 4.0), 'X');
        out->addCh((int) (y + height / 4.0), (int) (x + width / 4.0), 'X');
        out->addCh((int) (y - height / 4.0), (int) (x + width / 4.0), 'X');*/
        break;
      case BAD_PARTICLE:
        graphics->drawBadParticle(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, graphics->makeColor(1, 0, 0, 1), 0, "", true));
        //out->addCh((int) y, (int) x, '0');
        break;
    }
  }
}

void Bad::kill()
  // this just kills and does not deal with dropping powerups (only server calls dropPowerup)
{
  health = 0;
  dying = true;

  for (int i = 0; i < 4; i++) debris[i].setActive(true);

  char c = '?';

  switch (type) {
    case BAD_SHIP:
      c = 'X';
      break;
    case BAD_PARTICLE:
      c = '0';
      break;
  }

  Color col = { 1, 0, 0, 1};

  debris[0].set(x - width / 4.0, y - height / 4.0, -70, -70, 0, 0, col, c);
  debris[1].set(x - width / 4.0, y + height / 4.0, -70, 70, 0, 0, col, c);
  debris[2].set(x + width / 4.0, y + height / 4.0, 70, 70, 0, 0, col, c);
  debris[3].set(x + width / 4.0, y - height / 4.0, 70, -70, 0, 0, col, c);
}

/*void Bad::checkDead()
  // this is separate so can call clear() checkDead() draw()
{
  if (dead) active = false;
}*/ // TODO remove

void Bad::dropPowerup(Powerupcontrol &powerupcontrol, Net &net, Server &server)
{
#define POWERUP_SPEEDY 10

  if (powerup) {
    int powerupId = powerupcontrol.make(x, y, POWERUP_SPEEDY, powerupType);
    if (powerupId > -1) {
      // send out
      Unit unit;
      unit.powerup.flag = UNIT_POWERUP;
      unit.powerup.id = powerupId;
      unit.powerup.x = x;
      unit.powerup.y = y;
      unit.powerup.z = 0;
      unit.powerup.speedY = POWERUP_SPEEDY;
      unit.powerup.type = powerupType;
      unit.powerup.collected = -1; // not collected
      net.addUnitAll(unit, server, -1);
    }else *out << VERBOSE_LOUD << "Powerup overflow!\n";
  }
}

void Bad::checkCollision(int id, Bulletcontrol &bc, Powerupcontrol &pc, Net &net, Server &server)
{
  int hit = 0;

  Bullet* bullet = bc.getBullets();

  for (int i = 0; i < bulletlen; i++) {
    if (bullet[i].getActive() && !bullet[i].getDead() && bullet[i].getOwner() < ID_BAD_MIN &&
        bullet[i].getX() - bullet[i].getWidth() / 2.0 < x + width / 2.0 &&
        bullet[i].getX() + bullet[i].getWidth() / 2.0 > x - width / 2.0 &&
        bullet[i].getY() - bullet[i].getHeight() / 2.0 < y + height / 2.0 &&
        bullet[i].getY() + bullet[i].getHeight() / 2.0 > y - height / 2.0)
    {
      hit++;
      bullet[i].setDead(true);
      // when bullet.go() is called it will transmit that it's no longer active
    }
  }

  //if (hit > 0) out->addln("Hit!");
  
  health -= hit;

  if (health < 1 && !dying) {
    kill();
    dropPowerup(pc, net, server);
    Unit unit;
    unit.bad.flag = UNIT_BAD;
    unit.bad.id = id;
    unit.bad.type = -1;
    unit.bad.status = 2;
    net.addUnitAll(unit, server, -1);
  }
}

void Bad::backToFormation()
{
  mode = BAD_KAMIKAZEE_TO_FORMATION;
  // note move() will ignore mode change in this mode, it's set when it gets home()
}

void Bad::setHoldFormation(bool b)
{
  holdFormation = b;
}

void Bad::setHoldFire(bool b)
{
  holdFire = b;
}

void Bad::go(int id, float formationX, float formationY, float playerX, float playerY, double sync, Bulletcontrol &bulletcontrol, Powerupcontrol &powerupcontrol, Timer &timer, Net &net, Server &server)
{

  // will deactivate when all debris are inactive
  move(formationX, formationY, playerX, playerY, sync, timer);

  if (!dying) {
    fire(id, bulletcontrol, timer, net, server);

    // will set dying if hit and out of health
    // this initialises debris
    checkCollision(id, bulletcontrol, powerupcontrol, net, server); 
  }

  //checkDead(); // will deactivate if dead. Must be called independently by client after clear()
  
  // now checked dead in moveDebris so it's done locally as well
  /*else{
    // check if all debris are dead, then deactivate
    for (int i = 0; i < 4; i++) {
      if (!debris[i].getActive()) dead++;
    }
  }

  if (dead == 4) active = false;
  //else draw();*/
}

