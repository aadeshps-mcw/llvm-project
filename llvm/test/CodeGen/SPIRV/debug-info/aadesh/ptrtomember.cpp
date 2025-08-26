// ptr_to_member_simple.cpp

struct S {
  int a;
};

int S::*ptr = &S::a;

int use() {
  S s{42};
  return s.*ptr;  // use the pointer-to-member
}
