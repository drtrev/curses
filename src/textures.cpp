#include "textures.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "outverbose.h"

Textures::Textures()
{
  total = 0;
  for (int i = 0; i < textureslen; i++) id[i] = 0;
}

void Textures::init(Outverbose &o)
{
  out = &o;
}

int Textures::generate(int w, int h, int bpp, const unsigned char* data)
  // width, height and bytes per pixel (i.e. 3 for RGB, 4 for RGBA), returns openGL texture ID
{
  if (total < textureslen) {
    GLenum format = (bpp == 4) ? GL_RGBA : GL_RGB;

    glGenTextures(1, (GLuint*) &id[total]);
    *out << VERBOSE_LOUD << "assigned picture id: " << (int) id[total] << '\n';
    glBindTexture(GL_TEXTURE_2D, id[total]);
    //glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // TODO what is default?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST); // mipmap!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    *out << "bound\n";
    gluBuild2DMipmaps(GL_TEXTURE_2D, format, w, h, format, GL_UNSIGNED_BYTE, data);

    //glTexImage2D(GL_TEXTURE_2D, 0, bpp, w, h, 0, format, GL_UNSIGNED_BYTE, data);
     *out << "built\n";

    total++;

    return id[total-1];
  }else{
    *out << VERBOSE_LOUD << "Error, textures array full\n";
    return 0;
  }
}

int Textures::getId(int i)
  // gives texture id
{
  if (i > -1 && i < textureslen) return id[i];
  else return 0;
}

