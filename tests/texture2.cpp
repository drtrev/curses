#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
//#include <GL/glut.h>
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
  //glutInit(&argc, argv);

  int width = 640;
  int height = 480;
  int bpp = 24;
  int flags = SDL_OPENGL;

  //int window = glutCreateWindow ("Shoot-em-up Project");
  SDL_SetVideoMode( width, height, bpp, flags );

  int size = 256 * 256 * 3;

  char *data = new char[size];

  for (int i = 0; i < size; i++) {
    data[i] = 0;
  }

  GLuint id;
  glGenTextures(1, &id);

  cout << glGetError() << endl;
  cout << (unsigned)data << ' ' << id << endl;

  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); // mipmap!

  cout << "Boom" << endl;
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, data);
  cout << "Boom2" << endl;

  delete [] data;

}

