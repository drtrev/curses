#ifndef TEXTURES
#define TEXTURES

#define textureslen 10

class Outverbose;

class Textures {
  private:
    int total; // number of textures
    Outverbose* out;
    unsigned int id[textureslen];

  public:
    Textures();

    void init(Outverbose&);

    int generate(int, int, int, const unsigned char*);

    int getId(int);
};

#endif
