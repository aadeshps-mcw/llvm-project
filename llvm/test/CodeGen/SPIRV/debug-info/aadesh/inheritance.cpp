// simple_inheritance.cpp
class Base {
public:
    int x;
};

class Derived : public Base {
public:
    int y;
};

int main() {
    Derived d;
    d.x = 10;
    d.y = 20;
    return d.x + d.y;
}
