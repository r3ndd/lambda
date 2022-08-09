#include "input.hh"

#include <cstdio>
#include <iostream>
#include <istream>
#include <string>
#include <vector>

using namespace std;

bool Input::AtEnd() {
    if (!buffer.empty())
        return false;
    else
        return cin.eof();
}

char Input::UngetChar(char c) {
    if (c != EOF) buffer.push_back(c);
    return c;
}

void Input::GetChar(char &c) {
    if (!buffer.empty()) {
        c = buffer.back();
        buffer.pop_back();
    } else {
        cin.get(c);
    }
}

string Input::UngetString(string s) {
    for (unsigned int i = 0; i < s.size(); i++)
        buffer.push_back(s[s.size() - i - 1]);
    return s;
}
