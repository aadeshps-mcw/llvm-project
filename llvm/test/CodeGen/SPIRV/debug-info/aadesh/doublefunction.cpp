// Filename: simple_debug.cpp
//
// A small test file with a few debug info entries.
// It is designed to be easily digestible for testing the SPIRV
// non-semantic debug info extension.

// A global variable to test DIGlobalVariable.
int global_counter = 0;

// A small helper function to test DISubprogram.
int add_one(int value) {
    return value + 1;
}

// The main function with a local variable and a loop.
int main() {
    // A local variable to test DILocalVariable.
    int local_sum = 0;

    // A simple loop to generate multiple debug locations.
    for (int i = 0; i < 5; ++i) {
        local_sum += add_one(i);
        global_counter += 1;
    }

    return local_sum;
}