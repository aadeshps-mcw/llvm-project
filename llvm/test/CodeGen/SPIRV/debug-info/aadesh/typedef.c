// typedef_test.c

typedef int myint;  // This typedef will be translated into DebugTypedef

myint square(myint x) {
    return x * x;
}

int main() {
    myint val = 7;
    myint result = square(val);
    return result;  // Just return the result to use the typedef
}
