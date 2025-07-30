// file: import.cpp
#include "ns.h"

using namespace myns;

int main() {
    int a = 5;
    int b = double_it(a); // imported symbol from namespace
    return b;
}