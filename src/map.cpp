#include "map.h"
#include <boost/regex.hpp>
#include <cmath>
#include <fstream>
#include "graphics.h"
#include "input.h"
#include <iostream>
#include <string>

using std::string;
using std::vector;

Map::Map()
{
  accelX = 0, accelY = 0;
  oldAccelX = 0, oldAccelY = 0;
  friction = 1400; // 100 Hz, added in sync to power and friction
  speedX = 0, speedY = 0;
  minSpeed = 5;
  x = 0, y = 0;
  zoom = 0.01;
  power = 200000; // 120000;

  highestSync = 0; // temporary for error checking
}

//#define BUFFER_SIZE 10000

void Map::tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters)
  // stolen from http://www.oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

void Map::init(Outverbose &o, Graphics &g)
{
  out = &o;
  graphics = &g;

  //char *buf = new char[BUFFER_SIZE];

  vector <vector <double> > linev;
  vector <double> c(2, 0); // 2 doubles of value 0

  boost::regex linestr("LineString");
  boost::regex coordinates(".*coordinates>(.*)<");
  boost::match_results<std::string::const_iterator> match;
  string::const_iterator start, end; 
  string line, word;
  vector <string> tokens1, tokens2;
  enum Type { TYPE_NONE, TYPE_LINESTRING, TYPE_POINT };
  int type = TYPE_NONE;

  std::ifstream mapfile ("/tmp/trev/map2/map2.gml");

  if (!mapfile.is_open()) {
    std::cerr << "Error opening map file" << std::endl;
  }else{

    while (!mapfile.eof()) {

      getline(mapfile, line); // mapfile.getline(buf, BUFFER_SIZE - 1);

      if (!mapfile.eof()) {
        start = line.begin(); 
        end = line.end();
        if (boost::regex_search(line, linestr)) {
          type = TYPE_LINESTRING;
        }
        if (boost::regex_search(start, end, match, coordinates)) {
          //std::cerr << match[1] << std::endl;
          tokens1.clear();
          linev.clear();
          if (type == TYPE_LINESTRING) {
            type = TYPE_NONE;
            tokenize(match[1], tokens1);
            for (unsigned int i = 0; i < tokens1.size(); i++) {
              //std::cerr << tokens1[i] << std::endl;
              tokens2.clear();
              tokenize(tokens1[i], tokens2, ",");
              if (tokens2.size() == 2) {
                //std::cerr << "original: " << tokens2[0] << "," << tokens2[1] << std::endl;
                c[0] = atof(tokens2[0].c_str()) - 427672;
                c[1] = atof(tokens2[1].c_str()) - 432772;
                linev.push_back(c);
                //std::cerr << "c: " << linestrip.back()[0] << "," << linestrip.back()[1] << std::endl;
              }else{
                std::cerr << "Error with token thing, not two coords" << std::endl;
              }
            }
            linestrip.push_back(linev);
            if (linestrip.size() > 10000) {
              break;
            }
          }
          //break;
        }
      }

    } // end while

    std::cerr << "Linestrip size: " << linestrip.size() << std::endl;
  }

  mapfile.close();

  //delete [] buf;
}

void Map::draw()
{
  graphics->drawMap(linestrip, zoom, x, y);
  GraphicsInfo g = graphics->defaultInfo();
  g.x = -120, g.y = -120;
  g.scaleX = 0.5, g.scaleY = 0.5;
  g.text = "Zoom: ";
  char c[10];
  if (snprintf(c, 10, "%.2f", zoom) > 9) {
    c[9] = '\0';
  }
  g.text += c;
  graphics->drawText(g);
}

void Map::input(int in, double sync)
  // called by server
{
  if (sync > highestSync) {
    highestSync = sync;
    std::cerr << "highestSync: " << sync << std::endl;
  }
  if (sync > 0.05) {
    sync = 0.05;
  }
  if (in & KEYS_RIGHT) accelX += power * sync;
  if (in & KEYS_LEFT) accelX -= power * sync;
  if (in & KEYS_UP) accelY += power * sync;
  if (in & KEYS_DOWN) accelY -= power * sync;
  if (in & KEYS_ZOOM_IN) zoom += sync; /// 10.0;
  if (in & KEYS_ZOOM_OUT) zoom -= sync;// / 10.0;
  //if (in & KEYS_ZOOM_IN || in & KEYS_ZOOM_OUT) {
   // std::cerr << "x: " << x << ", y: " << y << std::endl;
   // std::cerr << "zoom: " << zoom << std::endl;
  //}
}

void Map::move(double sync)
{
  // create a dead zone, because the slow movement as we stop can cause a jump into a new text coord
  if (accelX == oldAccelX && fabs(speedX) < minSpeed) speedX = 0;
  if (accelY == oldAccelY && fabs(speedY) < minSpeed) speedY = 0;

  // this distance should be the integral of the velocity function, which can be
  // found on that drag site
  // hmm... acceleration's not constant. Need to find velocity function and then
  // can get approximate acceleration between last frame using a = (v - u) / t
  // Ok think I've got this now - basically calculating friction at tiny intervals
  // is the same as working out the integral (an approximation)
  //
  // s = ut + 0.5 * att
  // s is displacement. s equals ut plus a half a t-squared
  // u is initial velocity, t is time, using 0 as time started accelerating

  x += (speedX/zoom) * sync + 0.5 * accelX * sync * sync;
  y += (speedY/zoom) * sync + 0.5 * accelY * sync * sync;

  // update velocity
  // v = u + at
  speedX = speedX + accelX * sync;
  speedY = speedY + accelY * sync;

  // friction
  // this is a percentage of speed, i.e. the faster you move the more air resistance
  // this should really take into account time, see http://en.wikipedia.org/wiki/Drag_(physics)
  // This works here with sync, but friction is assumed constant over that time period,
  // so if it is running significantly slower then it will stop noticably quicker
  // Think integral approximations: need small time intervals to be more precise.
  accelX = -friction * speedX * sync;
  accelY = -friction * speedY * sync;

  oldAccelX = accelX; // may be modified in input
  oldAccelY = accelY;

  // could have boundary check here, see player.cpp
}

void Map::setX(float nx)
{
  x = nx;
}

void Map::setY(float ny)
{
  y = ny;
}

void Map::setZoom(float nz)
{
  zoom = nz;
}

float Map::getX() const
{
  return x;
}

float Map::getY() const
{
  return y;
}

float Map::getZoom() const
{
  return zoom;
}

