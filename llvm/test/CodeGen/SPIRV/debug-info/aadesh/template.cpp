// RUN: %clangxx -g -O0 -S -emit-llvm template_test.cpp -o - -std=c++17

template <typename T, int N>
struct FixedArray {
  T data[N];
};

FixedArray<int, 10> fa_global;

int main() {
  return 0;
}
