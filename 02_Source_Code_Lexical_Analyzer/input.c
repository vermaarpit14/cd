#include <stdio.h>

int main(void) {
    int count = 10;
    float ratio = 3.14e2;
    double small = .5;
    char letter = 'A';
    char newline = '\n';
    char *message = "Hello, \"world\"\n";
    /* block
       comment */
    for (int i = 0; i <= count; i++) {
        if (i != 5 && ratio >= 2.5f) {
            count += i << 2;
        }
    }
    return 0; // done
}
