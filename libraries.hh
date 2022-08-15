#ifndef __LIBRARIES_H__
#define __LIBRARIES_H__

#include <map>

using namespace std;

const string STDLIB =
    "import bool;"
    "import num;"
    "";

const string BOOL =
    "let true = !x.!y.x;"
    "let false = !x.!y.y;"
    "";

const string MATH = "";

const map<string, string> LIBRARIES = {
    {"stdlib", STDLIB}, {"bool", BOOL}, {"math", MATH}};

#endif