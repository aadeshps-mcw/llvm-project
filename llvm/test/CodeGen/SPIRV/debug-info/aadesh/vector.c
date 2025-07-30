typedef int v4i __attribute__((vector_size(16)));

void foo(v4i a) {
  v4i b = a + a;
}
