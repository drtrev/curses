#include "graphicsopengl.h"
#include <GL/gl.h>
//#include "windowgenglut.h"
#include "outverbose.h"
#include "windowgensdl.h"

GraphicsOpenGL::GraphicsOpenGL()
{
  frustum.near = 0.1;
  frustum.far = 1000;
  frustum.top = 10; // calculate properly when window is opened in init
  frustum.right = 10;

  border.top = 100, border.right = 100, border.bottom = 100, border.left = 100;
}

bool GraphicsOpenGL::init(Outverbose &o, WindowInfo w, const char* font, int fontsize)
{
  initShared(o);

  // initialise font
  face = new OGLFT::TranslucentTexture( font, fontsize );
  face->setHorizontalJustification( OGLFT::Face::CENTER );
  face->setVerticalJustification( OGLFT::Face::MIDDLE );
  //face->setForegroundColor( 0.75, 1., .75, 1. ); -- set in drawText
  /*face->setCharacterRotationX( 0 );
  face->setCharacterRotationY( 0 );
  face->setCharacterRotationZ( 0 );
  face->setStringRotation( 0 );*/

  //window = new WindowGlut;
  window = new WindowSDL;

  if (!window->init(*out, w)) return false;

  frustum.top = frustum.near; // make same as near plane for 90 degree vertical FOV
  frustum.right = frustum.top * (float) window->getWidth() / window->getHeight();

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glFrustum(-frustum.right, frustum.right, -frustum.top, frustum.top, frustum.near, frustum.far);

  glMatrixMode (GL_MODELVIEW);

  // initialise openGL
  glViewport(0, 0, window->getWidth(), window->getHeight());
  //*out << VERBOSE_LOUD << "width: " << window->getWidth() << ", height: " << window->getHeight() << '\n';

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //glCullFace(GL_FRONT); // if we mirror it
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  // typical usage: modify incoming colour by it's alpha, and existing colour by 1 - source alpha
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);

  return true; // success
}

void GraphicsOpenGL::kill()
{
  if (window->getWid() > -1) window->destroy();
  delete window;
  delete face;
}

void GraphicsOpenGL::drawStart()
{
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
  //glScalef(1, -1, 1);
  //glTranslatef(-out->getWidth(), -out->getWidth(), -out->getWidth()); // move camera out

  float zoom = 128;
  glTranslatef(0, 0, -zoom);

  if (window->getWidth() > window->getHeight()) {
    border.top = zoom;
    border.bottom = -border.top;
    border.right = border.top * window->getWidth() / (float) window->getHeight();
    border.left = -border.right;
  }

  // draw axis (temporarily to help)
  /*glColor4f(1, 1, 1, 1);
  glBegin(GL_LINES);
  glNormal3f(0, 0, 1);
  glVertex3f(0, -50, 0);
  glVertex3f(0, 50, 0);

  glVertex3f(-50, 0, 0);
  glVertex3f(50, 0, 0);
  glEnd();*/
}

void GraphicsOpenGL::flip()
// call pushMatrix and pushAttrib(GL_POLYGON_BIT) (for GL_CULL_FACE_MODE) first
{
  glCullFace(GL_FRONT);
  glScalef(2, -4, 1);
  glTranslatef(border.left / 2.0 + 2, -border.top / 4.0 + 3, 1);
}

void GraphicsOpenGL::drawBadShip(GraphicsInfo g)
{
  if (g.visible) {
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    flip();

    glColor4f(1, 0, 0, 1);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(g.x - g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y + g.height / 2.0, 0);
    glVertex3f(g.x - g.width / 2.0, g.y + g.height / 2.0, 0);
    glEnd();

    glPopAttrib();
    glPopMatrix();
  }
}

void GraphicsOpenGL::drawBadParticle(GraphicsInfo g)
{
  if (g.visible) {
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    flip();

    glColor4f(1, 0, 0, 1);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(g.x - g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y + g.height / 2.0, 0);
    glVertex3f(g.x - g.width / 2.0, g.y + g.height / 2.0, 0);
    glEnd();

    glPopAttrib();
    glPopMatrix();
  }
}

void GraphicsOpenGL::drawBullet(GraphicsInfo g)
{
  if (g.visible) {
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    flip();

    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(g.x - g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y + g.height / 2.0, 0);
    glVertex3f(g.x - g.width / 2.0, g.y + g.height / 2.0, 0);
    glEnd();

    glPopAttrib();
    glPopMatrix();
  }
}

void GraphicsOpenGL::drawDebris(GraphicsInfo g)
{
  if (g.visible) {
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    flip();

    glColor4f(g.color.red, g.color.green, g.color.blue, g.color.alpha);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(g.x - g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y + g.height / 2.0, 0);
    glVertex3f(g.x - g.width / 2.0, g.y + g.height / 2.0, 0);
    glEnd();

    glPopAttrib();
    glPopMatrix();
  }
}

void GraphicsOpenGL::drawMap(std::vector < std::vector < std::vector <double> > > &coords, double scale, double offsetX, double offsetY)
{
  /*static double oldscale = 0;
  if (oldscale != scale) {
    //std::cerr << "Scale: " << scale << std::endl;
    oldscale = scale;
    //std::cerr << "x: " << offsetX << "y: " << offsetY << std::endl;
  }*/

  glPushMatrix();
  glColor4f(1, 1, 1, 1);
  //glTranslatef(-coords[0][0][0] + 50, -coords[0][0][1], 0);
  //glTranslatef(500000, 0, 0);

  glScalef(scale, scale, 0);
  glTranslatef(1000, -500, 0);
  glTranslatef(offsetX, offsetY, 0);

  //glTranslatef(-500000 * (1/scale), 0, 0);
  //glTranslatef(coords[0][0][0] - 50, coords[0][0][1], 0);

  for (unsigned int line = 0; line < coords.size(); line++) {
    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i < coords[line].size(); i++) {
      glVertex3f(coords[line][i][0], coords[line][i][1], 0);
    }
    glEnd();
  }

  glPopMatrix();
}

void GraphicsOpenGL::drawPicture(GraphicsInfo g)
{
  float padding = 768 - g.height;
  if (padding < 0) padding = 0;
  g.height += padding;

  // normalise image size to space available (i.e. border size)
  // border size is the window size in openGL units, i.e. we're not measuring in pixels
  // try width first

  //*out << VERBOSE_LOUD << "window->getWidth(): " << window->getWidth() << ", window->getHeight(): " << window->getHeight()
  //<< ", g.width: " << (int) g.width << ", g.height: " << (int) g.height << '\n';
  //float width = (float) window->getWidth();
  //float height = g.height * (float) window->getWidth() / g.width;
  float width = border.right - border.left;
  float height = g.height * width / g.width;
  //*out << VERBOSE_LOUD << "width: " << (int) width << ", height: " << (int) height << "\n";

  //if (height > window->getHeight()) {
    // do height instead
    //height = (float) window->getHeight();
    //width = g.width * (float) window->getHeight() / g.height;
  //}

  if (height > border.top - border.bottom) {
    // do height instead
    height = border.top - border.bottom;
    width = g.width * height / g.height;
  }

  float sizeX = width / 2.0;
  float sizeY = height / 2.0;

  //*out << VERBOSE_LOUD << "width: " << (int) width << ", height: " << (int) height << '\n';

  glPushMatrix();

  glTranslatef(g.x, g.y - padding / g.height * (border.top - border.bottom) / 2.0, g.z);
  glTranslatef(0, 0, g.pivotZ); // change pivot point (was -300)
  glRotatef(g.angleX, 1, 0, 0);
  glRotatef(g.angleY, 0, 1, 0);
  glRotatef(g.angleZ, 0, 0, 1);
  glTranslatef(0, 0, -g.pivotZ); // change pivot point (was 300)
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, g.texture);
  
  // playing around
  glScalef(g.scaleX, g.scaleY, g.scaleZ);

  glColor4f(1, 1, 1, 1);
  glBegin(GL_QUADS);
  glNormal3f(0, 0, 1);
  glTexCoord2f(0, 1); glVertex3f(-sizeX, -sizeY, 0);
  glTexCoord2f(1, 1); glVertex3f(sizeX, -sizeY, 0);
  glTexCoord2f(1, 0); glVertex3f(sizeX, sizeY, 0);
  glTexCoord2f(0, 0); glVertex3f(-sizeX, sizeY, 0);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glPopMatrix();
}

void GraphicsOpenGL::drawPlayerShip(GraphicsInfo g)
{
  if (g.visible) {
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    flip();

    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(g.x - g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y + g.height / 2.0, 0);
    glVertex3f(g.x - g.width / 2.0, g.y + g.height / 2.0, 0);
    glEnd();

    glPopAttrib();
    glPopMatrix();
  }
}

void GraphicsOpenGL::drawPowerup(GraphicsInfo g)
{
  if (g.visible) {
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    flip();

    glColor4f(0, 1, 0, 1);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(g.x - g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y - g.height / 2.0, 0);
    glVertex3f(g.x + g.width / 2.0, g.y + g.height / 2.0, 0);
    glVertex3f(g.x - g.width / 2.0, g.y + g.height / 2.0, 0);
    glEnd();

    glPopAttrib();
    glPopMatrix();
  }
}

void GraphicsOpenGL::drawText(GraphicsInfo g)
{
  if (g.visible) {
    glPushMatrix();
    glTranslatef(g.x, g.y, g.z);
    glTranslatef(0, 0, g.pivotZ); // change pivot point
    glRotatef(g.angleX, 1, 0, 0);
    glRotatef(g.angleY, 0, 1, 0);
    glRotatef(g.angleZ, 0, 0, 1);
    glTranslatef(0, 0, -g.pivotZ); // change pivot point
    glScalef(0.5, 0.5, 0.5); // get higher resolution font by choosing large fontsize then halving rendering
    glScalef(g.scaleX, g.scaleY, g.scaleZ);
    glEnable(GL_TEXTURE_2D);

    face->setForegroundColor( g.color.red, g.color.green, g.color.blue, g.color.alpha );
    //face->draw(g.x, g.y, g.z, g.text.c_str());
    face->draw(0, 0, 0, g.text.c_str());

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
  }
}

void GraphicsOpenGL::drawStop()
{
  glPopMatrix();
}

void GraphicsOpenGL::refresh()
{
  window->refresh();
}

float GraphicsOpenGL::getWidth()
{
  return border.right - border.left;
}

