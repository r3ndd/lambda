#ifndef __LIBRARIES_H__
#define __LIBRARIES_H__

#include <map>

using namespace std;

const string STDLIB =
    "import bool;"
    "import num;"
    "";

const string BOOL =
    "let true = !a.!b.a;"
    "let false = !a.!b.b;"
    "";

const string MATH =
    "let succ = !n.!a.!b.a (n a b);"
    "let pred = !n.!a.!b.n (!c.!d.d (c a)) (!e.b) (!e.e);"
    "let add = !m.!n.!a.!b.m a (n a b);"
    "";

const map<string, string> LIBRARIES = {
    {"stdlib", STDLIB}, {"bool", BOOL}, {"math", MATH}};

#endif