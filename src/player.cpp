#include "player.h"
#include "bad.h"
#include "badcontrol.h"
#include "bullet.h"
#include "bulletcontrol.h"
#include <cmath>
#include "graphics.h"
#include "input.h"
#include "network/net.h"
#include "network/network.h"
#include "outverbose.h"
#include "powerupcontrol.h"
#include "timer.h"

Player::Player()
{
  startLives = 16;
  lives = startLives;
  accelX = 0, accelY = 0;
  oldAccelX = 0, oldAccelY = 0;
  //power = 1000;
  //friction = 16; // without loop delay
  friction = 1400; // 100 Hz, added in sync to power and friction
  //maxSpeed = 60; // this will be power / friction
  // (because if maxSpeed * friction = power then accel will be zero, so we want to find maxSpeed,
  // rearrange eq)
  speedX = 0, speedY = 0;
  minSpeed = 5;
  width = 3;
  height = 2;
  startX = 0, startY = 0; // set in init
  x = 0, y = 0;

  startHealth = 1;
  health = startHealth;
  dying = false;

  startBullets = 1;
  maxBullets = 10;
  bullets = startBullets;
  //bulletsActive = 0;

  //startPower = 800;
  //powerIncrement = 200; // speed powerup
  //maxPower = 1400;
  //power = startPower;
  startPower = 120000;
  powerIncrement = 20000; // speed powerup
  maxPower = 160000;
  power = startPower;

  lastFire.tv_sec = 0;
  lastFire.tv_usec = 0;

  deadTime.tv_sec = 0;
  deadTime.tv_usec = 0;

  //timeval start;
  //gettimeofday(&start, NULL);
  //startTime = start.tv_sec + start.tv_usec / 1000000.0;
}

Player::~Player()
{
}

void Player::init(Outverbose &o, Graphics &g)
{
  graphics = &g;
  out = &o;

  for (int i = 0; i < 4; i++) {
    debris[i].init(*out, *graphics);
  }

  startX = width / 2.0 + out->getWidth() / 2, startY = out->getHeight() - height / 2.0 - 5;

  x = startX;
  y = startY;
}

float Player::getX()
{
  return x;
}

float Player::getY()
{
  return y;
}

void Player::setX(float nx)
{
  x = nx;
}

void Player::setY(float ny)
{
  y = ny;
}

bool Player::getDying()
{
  return dying;
}

int Player::getLives()
{
  return lives;
}

timeval Player::getDeadTime()
{
  return deadTime;
}

void Player::local(double sync)
{
  if (dying) moveDebris(sync);
}

void Player::moveDebris(double sync)
  // note seperate function so can call locally
{
  for (int i = 0; i < 4; i++) {
    if (debris[i].getActive()) debris[i].move(sync);
  }
  // baddies check for deactivating here, but the player remains active and respawns on a time delay
  // (see servercontrol.cpp)
}

void Player::move(double sync)
{
  if (dying) {
    moveDebris(sync);
  }else{

    // create a dead zone, because the slow movement as we stop can cause a jump into a new text coord
    //if (accelX == -friction * speedX && fabs(speedX) < minSpeed) speedX = 0;
    //if (accelY == -friction * speedY && fabs(speedY) < minSpeed) speedY = 0;
    // now accel uses sync
    if (accelX == oldAccelX && fabs(speedX) < minSpeed) speedX = 0;
    if (accelY == oldAccelY && fabs(speedY) < minSpeed) speedY = 0;

    // this distance should be the integral of the velocity function, which can be
    // found on that drag site
    // hmm... acceleration's not constant. Need to find velocity function and then
    // can get approximate acceleration between last frame using a = (v - u) / t
    // Ok think I've got this now - basically calculating friction at tiny intervals
    // is the same as working out the integral (an approximation)
    //
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

    oldAccelX = accelX; // may be modified in input
    oldAccelY = accelY;

    // boundaries
    if (x - width / 2.0 < 0) x = width / 2.0;
    if (x + width / 2.0 > out->getWidth()) x = out->getWidth() - width / 2.0;
    if (y - height / 2.0 < 0) y = height / 2.0;
    if (y + height / 2.0 > out->getHeight()) y = out->getHeight() - height / 2.0;
  }
}

void Player::clear()
{
  if (dying) {
    for (int i = 0; i < 4; i++) {
      if (debris[i].getActive()) debris[i].clear();
    }
  }else{
    graphics->drawPlayerShip(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, graphics->makeColor(1, 1, 1, 1), 0, "", false));
    /*out->addCh((int) (y - height / 4.0), (int) x, ' ');
    out->addCh((int) (y + height / 4.0), (int) x, ' ');
    out->addCh((int) (y + height / 4.0), (int) (x - 1), ' ');
    out->addCh((int) (y + height / 4.0), (int) (x + 1), ' ');*/
  }
}

void Player::draw()
{
  if (dying) {
    for (int i = 0; i < 4; i++) {
      if (debris[i].getActive()) debris[i].draw();
    }
  }else{
    graphics->drawPlayerShip(graphics->makeInfo(x, y, 0, width, height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, graphics->makeColor(1, 1, 1, 1), 0, "", true));
    /*out->addCh((int) (y - height / 4.0), (int) x, '^');
    out->addCh((int) (y + height / 4.0), (int) x, 'X');
    out->addCh((int) (y + height / 4.0), (int) (x - 1), '^');
    out->addCh((int) (y + height / 4.0), (int) (x + 1), '^');*/
  }
}

void Player::collect(PowerupType type)
{
  switch (type) {
    case POWERUP_SPEED:
      power += powerIncrement;
      if (power > maxPower) power = maxPower;
      break;
    case POWERUP_BULLET:
      bullets++;
      if (bullets > maxBullets) bullets = maxBullets;
      break;
  }
}

void Player::checkCollision(int id, Bulletcontrol &bullc, Badcontrol &badc, Powerupcontrol &powerupc, timeval currentTime, Net &net, Server &server)
{
  int hit = 0;

  Unit unit;

  Bullet* bullet = bullc.getBullets();

  for (int i = 0; i < bulletlen; i++) {
    if (bullet[i].getActive() && !bullet[i].getDead() && bullet[i].getOwner() > ID_BAD_MIN - 1 &&
        bullet[i].getX() - bullet[i].getWidth() / 2.0 < x + width / 2.0 &&
        bullet[i].getX() + bullet[i].getWidth() / 2.0 > x - width / 2.0 &&
        bullet[i].getY() - bullet[i].getHeight() / 2.0 < y + height / 2.0 &&
        bullet[i].getY() + bullet[i].getHeight() / 2.0 > y - height / 2.0)
    {
      hit++;
      bullet[i].setDead(true);
    }
  }

  Bad* bad = badc.getBaddies();

  for (int i = 0; i < badlen; i++) {
    if (bad[i].getActive() && !bad[i].getDying() &&
        bad[i].getX() - bad[i].getWidth() / 2.0 < x + width / 2.0 &&
        bad[i].getX() + bad[i].getWidth() / 2.0 > x - width / 2.0 &&
        bad[i].getY() - bad[i].getHeight() / 2.0 < y + height / 2.0 &&
        bad[i].getY() + bad[i].getHeight() / 2.0 > y - height / 2.0)
    {
      bad[i].kill();
      bad[i].dropPowerup(powerupc, net, server);
      unit.bad.flag = UNIT_BAD;
      unit.bad.id = i;
      unit.bad.type = -1;
      unit.bad.status = 2;
      net.addUnitAll(unit, server, -1);
      hit++;
    }
  }

  health -= hit;

  if (health < 1 && !dying) {
    deadTime = currentTime;
    kill();

    badc.sendAllBackToFormation();
    badc.holdFormation(true);
    badc.holdFire(true);

    Unit unit;
    unit.player.flag = UNIT_PLAYER;
    unit.player.id = id;
    unit.player.status = 0; // kill
    net.addUnitAll(unit, server, -1);
  }

  // powerups
  Powerup* powerup = powerupc.getPowerups();

  for (int i = 0; i < poweruplen; i++) {
    if (powerup[i].getActive() && !powerup[i].getDead() &&
        powerup[i].getX() - powerup[i].getWidth() / 2.0 < x + width / 2.0 &&
        powerup[i].getX() + powerup[i].getWidth() / 2.0 > x - width / 2.0 &&
        powerup[i].getY() - powerup[i].getHeight() / 2.0 < y + height / 2.0 &&
        powerup[i].getY() + powerup[i].getHeight() / 2.0 > y - height / 2.0)
    {
      collect(powerup[i].getType());

      // send out
      unit.powerup.flag = UNIT_POWERUP;
      unit.powerup.id = i;
      unit.powerup.x = powerup[i].getX();
      unit.powerup.y = powerup[i].getY();
      unit.powerup.z = 0;
      unit.powerup.speedY = 0;
      unit.powerup.type = powerup[i].getType();
      unit.powerup.collected = id;
      net.addUnitAll(unit, server, -1);

      // technically could deactivate here, this was done to allow for a clear->deactivate->draw process,
      // but now we use a draw->refresh->clear->deactivate
      // bullet still uses set dead, because that causes the transmission, but in the case of
      // powerups the transmission is done above, and not executed in powerup.cpp if dead
      powerup[i].setDead(true);
    }
  }
}

void Player::kill()
{
  health = 0;
  dying = true;
  lives--; // for ghost/local copy when I do that

  for (int i = 0; i < 4; i++) debris[i].setActive(true);

  Color col = { 1, 1, 1, 1};

  debris[0].set(x, y - height / 4.0, 0, -100, 0, 0, col, '^');
  debris[1].set(x, y + height / 4.0, 0, 100, 0, 0, col, 'X');
  debris[2].set(x - 1, y + height / 4.0, -100, 0, 0, 0, col, '^');
  debris[3].set(x + 1, y + height / 4.0, 100, 0, 0, 0, col, '^');
}

void Player::fire(int id, Bulletcontrol &bulletcontrol, Timer &timer, Net &net, Server &server)
{
  int bulletsActive = 0;

  Bullet* bullet = bulletcontrol.getBullets();

  for (int i = 0; i < bulletlen; i++) {
    if (bullet[i].getActive() && bullet[i].getOwner() == id + ID_PLAYER_MIN) {
      bulletsActive++;
    }
  }

  timeval elapsed = timer.elapsed(lastFire);

  if ((elapsed.tv_sec > 0 || elapsed.tv_usec > 100000) && bulletsActive < bullets) {
    lastFire = timer.getCurrent();
    if (!bulletcontrol.make(x, y - height / 2.0, -1, 40, id + ID_PLAYER_MIN, net, server)) {
      *out << VERBOSE_LOUD << "Bullet overflow!\n";
    }
  }
}

void Player::respawnLocal()
{
  health = startHealth;
  bullets -= 2;
  if (bullets < startBullets) bullets = startBullets;

  power -= powerIncrement * 2;
  if (power < startPower) power = startPower;

  dying = false;

  lastFire.tv_sec = 0;
  lastFire.tv_usec = 0;

  accelX = 0, accelY = 0;
  speedX = 0, speedY = 0;

  // wait for server to send position
}

void Player::respawn(int id, Badcontrol &badc, Net &net, Server &server)
{
  respawnLocal();

  x = startX;
  y = startY;

  badc.holdFire(false); // resume fire
  badc.holdFormation(false);

  // send position first
  Unit unit;
  unit.position.flag = UNIT_POSITION;
  unit.position.id = id + ID_PLAYER_MIN;
  unit.position.x = x;
  unit.position.y = y;
  unit.position.z = 0;
  net.addUnitAll(unit, server, -1);

  unit.player.flag = UNIT_PLAYER;
  unit.player.id = id;
  unit.player.status = 1;
  net.addUnitAll(unit, server, -1);
}

void Player::reset(Badcontrol &badc)
{
  health = startHealth;
  bullets = startBullets;
  //bulletsActive = 0;
  power = startPower;

  lives = startLives;
  dying = false;

  accelX = 0, accelY = 0;
  speedX = 0, speedY = 0;

  x = startX;
  y = startY;

  lastFire.tv_sec = 0;
  lastFire.tv_usec = 0;

  deadTime.tv_sec = 0;
  deadTime.tv_usec = 0;

  badc.holdFire(false); // resume fire
  badc.holdFormation(false);
}

void Player::input(int id, int in, Bulletcontrol &bulletcontrol, Timer &timer, double sync, Net &net, Server &server)
  // must only be called once per frame
{
  if (!dying) {
    if (in & KEYS_RIGHT) accelX += power * sync;
    if (in & KEYS_LEFT) accelX -= power * sync;
    if (in & KEYS_DOWN) accelY += power * sync;
    if (in & KEYS_UP) accelY -= power * sync;
    if (in & KEYS_FIRE) fire(id, bulletcontrol, timer, net, server);
  }
}

