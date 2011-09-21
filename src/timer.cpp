#include "timer.h"
#include <cstdlib> // for NULL

Timer::Timer()
{
  gettimeofday(&last, NULL); // initialise last
}

timeval Timer::difference(timeval t1, timeval t2)
// performs t1 - t2
{
  timeval since;

  since.tv_sec = t1.tv_sec - t2.tv_sec;
  since.tv_usec = t1.tv_usec - t2.tv_usec;

  // get rid of negative microseconds
  if (since.tv_usec < 0) {
    since.tv_usec += 1000000;
    since.tv_sec --;
  }

  return since;
}

timeval Timer::elapsed(timeval t)
// calculate time elapsed since t
{
  return difference(current, t);
}

timeval Timer::getCurrent()
{
  return current;
}

double Timer::update()
// call every frame
// returns time since last frame
{
  gettimeofday(&current, NULL);

  // since last frame
  timeval since = difference(current, last);

  // calculate speed multiplier, i.e. if half time has passed compared to
  // expected value, then halve speed of movement
  //double mult = (since.tv_sec * 1000000 + since.tv_usec) / MICROSECS_PER_FRAME;

  // update last
  last = current;

  //return mult;

  // time since last frame in seconds
  double sinceDbl = since.tv_sec + since.tv_usec / 1000000.0;

  return sinceDbl;
}
