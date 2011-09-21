#include "picture.h"
#include "graphics.h"
#include <Magick++.h>
#include "outverbose.h"

Picture::Picture()
{
  x = 0, y = 0, z = -300, targetX = 0, targetY = 0, targetZ = -300, width = 10, height = 10;
  angleX = 0, angleY = 180, angleZ = 0, targetAngleY = 180;
  pivotX = 0, pivotY = 0, pivotZ = -300;
  scaleX = 1, scaleY = 1, scaleZ = 1;
  offsetX = 0, offsetY = 0, offsetZ = 0;
  increasingZoom = true;
  increasingOffset = true;
  speed = 200;
  speedRot = 90;
  texId = 0;
  active = false;
  requested = false;
  loaded = 0; // number of clients who have it loaded
  direction = 1; // clockwise
}

void Picture::init(Outverbose &o, Graphics &g, Textures &t)
{
  out = &o;
  graphics = &g;
  textures = &t;
  Magick::InitializeMagick(NULL);
  *out << VERBOSE_NORMAL << "Initialised Magick.\n";
  /*if (strlen(p) < MAX_FILENAME_SIZE - 2) {
    path = p;
    // add on trailing slash if necessary
    if (path.size() > 0 && path[path.size()-1] != '/') {
      path += '/';
    }
  }else{
    *out << VERBOSE_LOUD << "Error, path too long, size: " << strlen(p) << '\n';
  }*/
}

void Picture::load(std::string filename)
{
  *out << VERBOSE_LOUD << "Loading picture: " << filename << "...\n";

  Magick::Image image;

  *out << VERBOSE_LOUD << "image object made. cstr: " << filename.c_str() << "\n";

  try {
    image.read(filename.c_str());

    *out << "read complete\n";

    *out << "cols: " << (int) image.columns() << ", rows: " << (int) image.rows() << '\n';
    image.getConstPixels(0,0,image.columns(),image.rows()); // copy to the pixel cache

    int padding = 768 - image.rows();
    if (padding < 0) padding = 0;

    unsigned char* data = new unsigned char[image.columns() * (padding + image.rows()) * 4];
    for (unsigned int i = 0; i < image.columns() * (padding + image.rows()) * 4; i++) data[i] = 0;

    image.writePixels(Magick::RGBAQuantum, data);
    *out << "Allocated\n";
    *out << "cols: " << (int) image.columns() << ", rows: " << (int) image.rows() << '\n';
    *out << "padding: " << padding << '\n';

    //imageLoad("../res/test.bmp", &im);
    texId = textures->generate(image.columns(), image.rows() + padding, 4, data);
    //textures.generate(256, 256, 3, im.data);

    width = image.columns();
    height = image.rows();
    
    delete [] data;
    //delete [] im.data;
  }
  catch (Magick::Exception &error_)
  {
    *out << VERBOSE_LOUD << "Caught exception: " << error_.what() << '\n';
  }
}

void Picture::incrementLoaded()
{
  loaded++;
  *out << VERBOSE_LOUD << "Picture loaded by a client\n";
}

void Picture::setActive(bool b)
{
  active = b;
}

bool Picture::getActive()
{
  return active;
}

void Picture::setRequested(bool b)
{
  requested = b;
}

bool Picture::getRequested()
{
  return requested;
}

int Picture::getWidth()
{
  return width;
}

int Picture::getHeight()
{
  return height;
}

float Picture::getX()
{
  return x;
}

float Picture::getY()
{
  return y;
}

float Picture::getZ()
{
  return z;
}

float Picture::getAngleY()
{
  return angleY;
}

void Picture::setX(float nx)
{
  x = nx;
}

void Picture::setY(float ny)
{
  y = ny;
}

void Picture::setZ(float nz)
{
  z = nz;
}

void Picture::setAngleY(float ay)
{
  angleY = ay;
}

void Picture::setSpeedRot(float s)
{
  speedRot = s;
}

void Picture::setTargetX(float tx)
{
  targetX = tx;
}

void Picture::setTargetZ(float tz)
{
  targetZ = tz;
}

void Picture::setTargetAngleY(float ty)
{
  targetAngleY = ty;
}

void Picture::setDirection(int d)
// can set to only rotate clockwise or anticlockwise here
{
  direction = d;
}

void Picture::loopAngles()
{
  *out << VERBOSE_LOUD << "looping angles: angleY from " << (int) angleY;
  if (angleY >= 360) angleY -= 360 * ((int) angleY / 360);
  if (angleY < 0) angleY += 360 * ((int)-angleY / 360 + 1);
  *out << " to " << (int) angleY << '\n';
}

void Picture::move(double sync)
// TODO use vector angle and accelDecel
{
  //*out << VERBOSE_LOUD << "targetX: " << (int) targetX << ", x: " << (int) x << '\n';
  if (x < targetX) {
    x += speed * sync;
    if (x > targetX) x = targetX;
  }
  if (x > targetX) {
    x -= speed * sync;
    if (x < targetX) x = targetX;
  }

  if (z < targetZ) {
    z += speed * sync;
    if (z > targetZ) z = targetZ;
  }
  if (z > targetZ) {
    z -= speed * sync;
    if (z < targetZ) z = targetZ;
  }

  // restrict to one direction of rotation
  // if use this then call loopAngles from picturecontrol
  /*if (direction == 1) {
    // clockwise
    if (angleY < targetAngleY) targetAngleY -= 360;
  }else{
    if (angleY > targetAngleY) targetAngleY += 360;
  }*/

  if (angleY < targetAngleY) {
    angleY += speedRot * sync;
    if (angleY > targetAngleY) angleY = targetAngleY;
  }
  if (angleY > targetAngleY) {
    angleY -= speedRot * sync;
    if (angleY < targetAngleY) angleY = targetAngleY;
  }

  // modify width/height
  /*static int maxwidth = 1224, minwidth = 800;
  static float speed = 100;
  static float maxScaleX = 2, minScaleX = 1;
  static bool increasing = true;
  float aspect = (float) texturewidth / textureheight;
  if (increasing) {
    width += (int) (speed * sync);
    height = (int) (width / aspect);
    if (width > maxwidth) increasing = false;
  }else{
    width -= (int) (speed * sync);
    height = (int) (width / aspect);
    if (width < minwidth) increasing = true;
  }*/

  //static bool increasingZoom = true;//, increasingOffset = true;
  //static bool increasingOffset = true;
  // didn't work when increasingOffset was static... maybe that would make it shared between members?
  float zoomSpeed = 0.02, maxScale = 1.7, minScale = 1;
  float offsetSpeed = 2, maxOffset = 10;
  maxOffset = ((1024 * scaleX - 1024) / 8.0);
  float minOffset = -maxOffset;
  //*out << VERBOSE_LOUD << "maxOffset: " << (int) maxOffset << '\n';
  //if (texId == 1) {
    //*out << VERBOSE_LOUD << "minOffset " << (int) minOffset << " offsetX: " << (int) offsetX << " scaleX: " << (int) scaleX << '\n';
  //}
  if (increasingZoom) {
    scaleX += zoomSpeed * sync;
    if (scaleX > maxScale) {
      scaleX = maxScale;
      increasingZoom = false;
    }
    scaleY = scaleX;
  }else{
    scaleX -= zoomSpeed * sync;
    if (scaleX < minScale) {
      scaleX = minScale;
      increasingZoom = true;
    }
    scaleY = scaleX;
  }
  if (increasingOffset) {
    offsetX += offsetSpeed * sync;
    if (offsetX * scaleX > maxOffset) {
      //offsetX = maxOffset * scaleX;
      increasingOffset = false;
    }
    offsetY = offsetX / 2.0;
  }else{
    offsetX -= offsetSpeed * sync;
    if (offsetX * scaleX < minOffset) {
      //offsetX = minOffset * scaleX;
      //if (texId == 1) *out << VERBOSE_LOUD << "ho!" << '\n';
      increasingOffset = true;
    }
    offsetY = offsetX / 2.0;
  }
}

void Picture::draw(int players)
{
  GraphicsInfo g = { x + offsetX / scaleX, y + offsetY / scaleY, z, width, height, 0, angleX, angleY, angleZ, pivotX, pivotY, pivotZ, scaleX, scaleY, scaleZ, graphics->makeColor(1, 1, 1, 1), texId, "", true };
  if (active) {
    graphics->drawPicture(g);
  }
  if (loaded < players) {
    g.text = "Loading...";
    //*out << VERBOSE_LOUD << "Players: " << players << ", loaded: " << loaded << '\n';
    g.z += 20;
    g.pivotZ -= 20;
    graphics->drawText(g);
  }
  if (loaded > players) {
    g.text = "Error!";
    g.z += 20;
    g.pivotZ -= 20;
    graphics->drawText(g);
    *out << VERBOSE_LOUD << "Error, loaded is more than players\n";
  }
}

