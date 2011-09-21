#include "inputSDL.h"
#include <iostream>

void InputSDL::grab()
{
  grabbed = true;
}

int InputSDL::map(SDLKey sym)
{
  int key = 0;

  switch (sym) {
    case SDLK_UP:
      key = KEYS_UP;
      break;
    case SDLK_LEFT:
      key = KEYS_LEFT;
      break;
    case SDLK_RIGHT:
      key = KEYS_RIGHT;
      break;
    case SDLK_DOWN:
      key = KEYS_DOWN;
      break;
    case SDLK_LCTRL:
      key = KEYS_FIRE;
      break;
    case SDLK_SPACE:
      key = KEYS_TALK;
      break;
    case SDLK_b:
      key = KEYS_BACK;
      break;
    case SDLK_l:
      key = KEYS_ZOOM_OUT;
      break;
    case SDLK_n:
      key = KEYS_NEXT;
      break;
    case SDLK_p:
      key = KEYS_ZOOM_IN;
      break;
    case SDLK_q:
      key = KEYS_QUIT;
      break;
    default:
      break;
  }

  return key;
}

int InputSDL::check(int keys)
{
  int key = 0;

  // see: http://www.libsdl.org/intro.en/usingevents.html
  //SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

  SDL_Event event;

  // TODO this loop will get press and release - could process each event separately

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_KEYDOWN:
        key = map(event.key.keysym.sym);
        keys |= key;
        break;
      case SDL_KEYUP:
        key = map(event.key.keysym.sym);
        keys &= ~key;
        break;
      case SDL_VIDEORESIZE:
        std::cerr << "Resize event received" << std::endl;
        // TODO call window.resize(event.w, event.h);
        break;
    }
  }

  return keys;
}

void InputSDL::release()
{
  grabbed = false;
}
