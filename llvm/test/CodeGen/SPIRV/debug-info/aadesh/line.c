int sum_up_to(int n) {
    int total = 0;
    for (int i = 0; i < n; ++i) {
        total += i;
    }
    return total;
}

int main() {
    int result = sum_up_to(10);
    return 0;
}