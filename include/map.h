#ifndef MAP
#define MAP

#include <string>
#include <vector>

class Graphics; class Outverbose;

class Map {
  private:
    Outverbose* out;
    Graphics* graphics;
    std::vector < std::vector < std::vector<double> > > linestrip;

    float x, y, accelX, accelY, power, friction, speedX, speedY, minSpeed;
    float oldAccelX, oldAccelY;
    float zoom;

    double highestSync;
  
  public:
    Map();

    void init(Outverbose&, Graphics&);
    
    void tokenize(const std::string&, std::vector<std::string>&, const std::string& delimiters = " ");

    void draw();

    void input(int, double);

    void move(double);

    void setX(float);
    void setY(float);
    void setZoom(float);
    float getX() const;
    float getY() const;
    float getZoom() const;
};

#endif
