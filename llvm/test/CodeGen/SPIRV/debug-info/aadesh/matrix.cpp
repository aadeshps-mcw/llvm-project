enum Color {
    Red = 1,
    Green = 2,
    Blue = -4
};

Color c = Red;

int main() {
    return static_cast<int>(c);
}
