#ifndef INPUT_SDL
#define INPUT_SDL

#include "input.h"
#include <SDL/SDL.h>

class InputSDL : public Input {
  private:
    int map(SDLKey);

  public:
    void grab();

    int check(int);

    void release();

};

#endif
