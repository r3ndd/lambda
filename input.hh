#ifndef __INPUT_H__
#define __INPUT_H__

#include <fstream>
#include <string>
#include <vector>

using namespace std;

class Input {
   public:
    bool OpenFile(string filename);
    void GetChar(char& c);
    char UngetChar(char c);
    string UngetString(string s);
    bool AtEnd();

   private:
    ifstream file;
    vector<char> buffer;
};

#endif
