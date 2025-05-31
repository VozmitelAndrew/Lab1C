#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char echo_for[500];

    if (fgets(echo_for, sizeof(echo_for), stdin) != NULL) {
        echo_for[strcspn(echo_for, "\n")] = '\0';
        printf("ECHO: %s\n", echo_for);
    } else {
        printf("ECHO: (no input)\n");
    }

    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    return 0;
}