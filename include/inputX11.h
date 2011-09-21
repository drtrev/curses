#ifndef INPUT_X11
#define INPUT_X11

// Inspiration from: http://www.xmission.com/~georgeps/documentation/tutorials/Xlib_Beginner.html

#include "input.h"
#include <X11/Xlib.h>

class InputX11 : public Input {
  private:
    Display *dis;
    Window win;
    XEvent report;

    int map(int);

  public:
    void grab(); // grab control of keyboard

    int check(int);

    void release(); // release control of keyboard
};

#endif
