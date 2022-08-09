#include "parser.hh"

int main() {
    Parser parser;
    parser.ParseInput();
    parser.ReduceAndPrint();
}