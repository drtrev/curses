#ifndef TIME
#define TIME

#include <sys/time.h>

#define MICROSECS_PER_FRAME 16666.6666667 // 60 FPS assumed, * 1000000
#define SECS_PER_FRAME 0.0166666666667

class Timer {
  private:
    // time of last frame
    // current time
    timeval last, current;

  public:
    Timer();

    timeval difference(timeval, timeval); // t1 - t2
    timeval elapsed(timeval); // calculate time elapsed since timeval
    timeval getCurrent();

    double update(); // call every frame, returns speed multiplier
};

#endif
