#include "directory.h"
#include <cstring>
#include "dirent.h"
#include <iostream>
#include "network/network.h"

using std::vector;
using std::string;
using std::cerr;
using std::endl;

vector <string> Directory::getPics(string path)
{
  // read directory path

  vector <string> out;
  DIR *dp;
  struct dirent *ep;

  dp = opendir(path.c_str());

  if (dp != NULL) {
    cerr << "Reading dir: " << path << endl;

    while ((ep = readdir(dp))) {
      //cerr << "Read file: " << ep->d_name << endl;

      if (std::strcmp(ep->d_name, ".") && std::strcmp(ep->d_name, "..")) {
        // if not current or parent directory

        /*string temp = path + ep->d_name; // add on path

        if (temp.size() < MAX_FILENAME_SIZE) {
          cerr << "Got file: " << temp << endl;
          out.push_back(temp);
        }else cerr << "Filename too big: " << temp << endl;*/

        if (strlen(ep->d_name) + 1 < MAX_FILENAME_SIZE) {
          out.push_back(ep->d_name);
          cerr << "Got file: " << out.back() << endl;
        }
      }
    }
    closedir(dp);
  }else{
    cerr << "Error with getPics, path not specified" << endl;
  }

  //out.push_back("../res/mountain.jpg");
  //out.push_back("../res/test.png");

  for (int i = 0; i < (int) out.size(); i++) {
    cerr << out[i] << endl;
  }

  return out;
}

