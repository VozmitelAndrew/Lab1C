#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("Hello from child bin!\n");

    printf("Total arguments: %d\n", argc);

    // Печатаем все аргументы
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    return 0;
}