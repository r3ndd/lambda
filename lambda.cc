#include <iostream>

#include "parser.hh"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Error: Include a local file name to interpret" << endl;
        exit(1);
    }

    Parser parser;
    parser.OpenFile(argv[1]);
    parser.ParseInput();
    parser.ReduceAndPrint();
}