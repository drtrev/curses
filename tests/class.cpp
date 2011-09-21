#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

class Test {
  public:

    void func(int a) {
      cout << "base class, a == " << a << endl;
    }

    virtual void func2(int a, int b) = 0;
};

class Testsub : public Test {
  public:
    void func(int b) {
      Test::func(b);
      cout << "subclass, b == " << b << endl;
    }

    // don't need virtual keyword here, but I guess if you want to subclass it again
    // then you'll need it if you want to re-implement this function
    virtual void func2(int a, int b) {
      cout << "a and b" << endl;
    }
};

int main(int argc, char** argv)
{
  Test *test = new Testsub;

  test->func(5);
  test->func2(5, 2);

  return 0;
}
