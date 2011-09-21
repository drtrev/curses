#include "windowgensdl.h"
#include "outverbose.h"
//#include <GL/gl.h>
#include <SDL/SDL.h>
#include <string>

bool WindowSDL::init(Outverbose& o, WindowInfo w)
{
  initShared(o, w);

  winfo = w;

  const SDL_VideoInfo* vinfo = NULL;
  int width = 0;
  int height = 0;
  int bpp = 0;
  int flags = 0;

  // stop it sefaulting on my ASUS EEE PC
  std::string tempenv = "SDL_VIDEO_X11_WMCLASS=curses";
  char ctempenv[50];
  strncpy(ctempenv, tempenv.c_str(), tempenv.length() + 1);
  SDL_putenv(ctempenv);

  *out << VERBOSE_QUIET << "Initialising SDL video...\n";

  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    *out << VERBOSE_LOUD << "Error initialising video with SDL: " << SDL_GetError() << '\n';
    return false;
  }

  vinfo = SDL_GetVideoInfo( );

  if( !vinfo ) {
    *out << VERBOSE_LOUD << "Error getting video vinfo: " << SDL_GetError() << '\n';
    return false;
  }

  /*
   * Set our width/height to 640/480 (you would
   * of course let the user decide this in a normal
   * app). We get the bpp we will request from
   * the display. On X11, VidMode can't change
   * resolution, so this is probably being overly
   * safe. Under Win32, ChangeDisplaySettings
   * can change the bpp.
   */
  if (w.fullscreen) {
    //width = 1280;
    width = vinfo->current_w;
    winfo.width = width;
    //height = 1024;
    height = vinfo->current_h;
    winfo.height = height;
  }else{
    width = 600;
    winfo.width = width;
    height = 400;
    winfo.height = height;
  }

  bpp = vinfo->vfmt->BitsPerPixel;

  /*
   * Now, we want to setup our requested
   * window attributes for our OpenGL window.
   * We want *at least* 5 bits of red, green
   * and blue. We also want at least a 16-bit
   * depth buffer.
   *
   * The last thing we do is request a double
   * buffered window. '1' turns on double
   * buffering, '0' turns it off.
   *
   * Note that we do not use SDL_DOUBLEBUF in
   * the flags to SDL_SetVideoMode. That does
   * not affect the GL attribute state, only
   * the standard 2D blitting setup.
   */
  if (bpp > 23) {
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  }else{
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
  }
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  //flags = SDL_OPENGL | SDL_FULLSCREEN;
  flags = SDL_OPENGL | SDL_RESIZABLE;
  if (w.fullscreen) flags |= SDL_FULLSCREEN;

  if( SDL_SetVideoMode( width, height, bpp, flags ) == 0 ) {
    *out << VERBOSE_LOUD << "Failed to set video mode: " << SDL_GetError() << '\n';
    return false;
  }

  return true;
}

void WindowSDL::refresh()
{
  SDL_GL_SwapBuffers();
}

void WindowSDL::destroy()
{
  SDL_Quit();
}

