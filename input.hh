#ifndef __INPUT_H__
#define __INPUT_H__

#include <string>
#include <vector>

class Input {
   public:
    void GetChar(char &);
    char UngetChar(char);
    std::string UngetString(std::string);
    bool AtEnd();

   private:
    std::vector<char> buffer;
};

#endif
