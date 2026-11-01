int main() {
    int count = 10;
    float ratio = 3.14;
    char letter = 'A';
    char *message = "Hello";
    for (int i = 0; i <= count; i++) {
        if (i != 5 && ratio >= 2.5) {
            continue;
        } else {
            count += i;
        }
    }
    while (count > 0) {
        count--;
        if (count == 3) break;
    }
    return 0;
}
