#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>

using namespace std;

struct Image2 {
  unsigned long sizeX;
  unsigned long sizeY;
  char *data;
};

int imageLoad(char* filename, Image2 *image);
void generate(int w, int h, int bpp, const char* data);

int main(int argc, char** argv)
{
  glutInit(&argc, argv);

  cout << "Loading..." << endl;

  Image2 im;

  //imageLoad("../res/test.bmp", &im);
  //generate(256, 256, 3, im.data);

  //for (int y = 0; y < 256; y++) {
    //for (int x = 0; x < 256*3; x++) {
     //im.data[y*256*3+x] = 0;
    //}
  //}

  im.data = new char[256*256*3];
  for (int i = 0; i < 256*256*3; i++) im.data[i] = 0;

  GLuint id;
  glGenTextures(1, &id); // TODO
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); // mipmap!
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, im.data);
  /*if (png[x] && bytesPerPixel == 4) {
    //cout << "Got some alpha" << endl;
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, tempTex->sizeX, tempTex->sizeY, GL_RGBA, GL_UNSIGNED_BYTE, tempTex->data);
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, true); requires power of 2 image
  }else
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tempTex->sizeX, tempTex->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tempTex->data);*/

  delete [] im.data;
}

int imageLoad(char* filename, Image2 *image)
{
  FILE *file;
  unsigned long size;                 // size of the image in bytes.
  unsigned long i;                    // standard counter.
  unsigned short int planes;          // number of planes in image (must be 1)
  unsigned short int bpp;             // number of bits per pixel (must be 24)
  char temp;                          // temporary color storage for bgr-rgb conversion.

  // make sure the file is there.
  if ((file = fopen(filename, "rb"))==NULL)
  {
	  printf("File Not Found : %s\n",filename);
	  return 0;
  }

  // seek through the bmp header, up to the width/height:
  fseek(file, 18, SEEK_CUR);

  // read the width
  if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
	  printf("Error reading width from %s.\n", filename);
	  return 0;
  }
  //printf("Width of %s: %lu\n", filename, image->sizeX);

  // read the height
  if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
	  printf("Error reading height from %s.\n", filename);
	  return 0;
  }
  //printf("Height of %s: %lu\n", filename, image->sizeY);

  // calculate the size (assuming 24 bits or 3 bytes per pixel).
  size = image->sizeX * image->sizeY * 3;
  cout << "sizeX: " << (int) image->sizeX << ", sizeY: " << (int) image->sizeY << endl;

  // read the planes
  if ((fread(&planes, 2, 1, file)) != 1) {
	  printf("Error reading planes from %s.\n", filename);
	  return 0;
  }
  if (planes != 1) {
	  printf("Planes from %s is not 1: %u\n", filename, planes);
	  return 0;
  }

  // read the bpp
  if ((i = fread(&bpp, 2, 1, file)) != 1) {
	  printf("Error reading bpp from %s.\n", filename);
	  return 0;
  }
  if (bpp != 24) {
	  printf("Bpp from %s is not 24: %u\n", filename, bpp);
	  return 0;
  }

  // seek past the rest of the bitmap header.
  fseek(file, 24, SEEK_CUR);

  // read the data.
  //image->data = (char *) malloc(size);
  image->data = new char[size];
  cout << "Size: " << size << endl;
  if (image->data == NULL) {
	  printf("Error allocating memory for color-corrected image data");
	  return 0;
  }

  if ((i = fread(image->data, size, 1, file)) != 1) {
	  printf("Error reading image data from %s.\n", filename);
	  return 0;
  }

  for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
	  temp = image->data[i];
	  image->data[i] = image->data[i+2];
	  image->data[i+2] = temp;
  }

  // we're done.
  return 1;
}

void generate(int w, int h, int bpp, const char* data)
  // width, height and bytes per pixel (i.e. 3 for RGB, 4 for RGBA)
{
  /*FILE *f = fopen("/tmp/pic", "w");
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w*3; j+=3) {
      fprintf(f, "%i,%i,%i,  ", data[i*w*3+j], data[i*w*3+j+1], data[i*w*3+j+2]);
    }
    fprintf(f, "\n");
  }
  fclose(f);*/

  int total = 0;
  int textureslen = 10;
  unsigned int id[10];

  if (total < textureslen) {
    GLenum format = (bpp == 4) ? GL_RGBA : GL_RGB;
    format = GL_RGB; // TODO

    //glEnable(GL_TEXTURE_2D); // TODO remove

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // TODO remove

    glGenTextures(1, (GLuint*) &id[total]); // TODO
    //glGenTextures(1, (GLuint*) id);
    cout << "assigned id: " << id[total] << '\n';
    glBindTexture(GL_TEXTURE_2D, id[total]);
    //glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // TODO what is default?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST); // mipmap!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);*/
    cout << "bound\n";
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, format, GL_UNSIGNED_BYTE, data);
    //gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 256, 256, format, GL_UNSIGNED_BYTE, data);
    //*out << "built\n";

    //glTexImage2D(GL_TEXTURE_2D, 0, bpp, w, h, 1, format, GL_UNSIGNED_BYTE, data);
    cout << "built\n";

    total++;
  }else{
    cout << "Error, textures array full" << endl;
  }

  cout << endl;
}
