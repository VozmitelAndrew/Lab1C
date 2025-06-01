#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>


#define MAX_ARG_SIZE 10
#define BUFFER_SIZE 256
#define MAX_SPLIT_AMOUNT 100



//Получает указатель на массив чаров
//Работает прямо поверх массива строк
void trim(char *s) {
    char *dest = s;
    while (*s != '\0') {
        if (!isspace(*s) || (s == dest || !isspace(*(dest - 1)))) {
            *dest++ = *s;
        }
        s++;
    }
    *dest = '\0';
}


char* strsepparator(char ** string, const char * delim){
    char *s = *string;
    if (!s) return NULL;

    char *p = s;
    while (*p) {
        if (*p == *delim) {
            *p = '\0';
            *string = p + 1;
            return s;
        }
        p++;
    }

    // Разделитель не найден - всё выплёвывается
    *string = NULL;
    return s;
}


//токенайзер получает строку и буффер для записи количества команд
//возвращает указатель на массив распаршенных строк
char ***tokenize(char *str, int *num_commands) {
    if (!str || *str == '\0') {
        *num_commands = 0;
        return NULL;
    }

    *num_commands = 1;
    for (char *p = str; *p; p++) {
        if (*p == '|') (*num_commands)++;
    }


    char ***commands = malloc(*num_commands * sizeof(char **));
    if (!commands) return NULL;

    //ВОТ ТУТ КАКАЯ-ТО ЧУШЬ ПОШЛА

    char *segment = str;
    for (int i = 0; i < *num_commands; i++) {
        char *piece = strsepparator(&segment, "|");
        trim(piece);

        int argc = 0;
        char *start = piece;
        while (*start) {
            while (*start && isspace((unsigned char)*start)) start++;
            if (!*start) break;
            argc++;
            while (*start && !isspace((unsigned char)*start)) start++;
        }


        char **argv = malloc((argc + 1) * MAX_ARG_SIZE * sizeof(char *));
        if (!argv) continue;

        int idx = 0;
        char *token;
        while ((token = strsepparator(&piece, " ")) != NULL) {
            if (*token) {
                argv[idx++] = token;
            }
        }
        argv[idx] = NULL;
        commands[i] = argv;
    }

    return commands;
}


int main(int argc, char *argv[]) {

    char buffer[BUFFER_SIZE];
    const char *exit_msg = "\nЗавершение работы...\n";


    while (1) {
        write(STDOUT_FILENO, "Write your prog, X to exit \n", strlen("Write your prog, X to exit \n"));

        //считываем с Input fd, при виде X завершаем работу
        if (read(STDIN_FILENO, buffer, sizeof(buffer)) == 0 || buffer[0] == 'X') {
            write(STDOUT_FILENO, exit_msg, strlen(exit_msg));
            break;
        }

        //формализую строку
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        //токенезация
        int token_count;
        char *trimmed = buffer;
        trim(trimmed);
        char ***tokens = tokenize(trimmed, &token_count);
        int pipe_links[token_count][2];
        for (int i = 0; i < token_count - 1; i++) {
            if (pipe(pipe_links[i]) < 0) {
                perror("pipe construction failed");
                exit(EXIT_FAILURE);
            }
        }

        pid_t pids[MAX_SPLIT_AMOUNT] = {0};


        for (int i = 0; i < token_count; ++i) {
            //printf("SO I AM GONNA EXECUTE amount %d\n", token_count);

            pid_t pid = fork();
            if (pid == -1) {
                perror("forked with error");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
                //Child process
                //printf("FORKED! \n");
                write(STDOUT_FILENO, "FORKED! \n", strlen("FORKED! \n"));
                char path[128] = "bin/";
                strcat(path, tokens[i][0]);
                //printf("SO I AM GONNA EXECUTE %s\n", tokens[i][0]);

                if (i > 0) {
                    dup2(pipe_links[i - 1][0], STDIN_FILENO);
                }

                if (i < token_count) {
                    dup2(pipe_links[i][1], STDOUT_FILENO);
                }


                for (int j = 0; j < token_count - 1; j++) {
                    close(pipe_links[j][0]);
                    close(pipe_links[j][1]);
                }

                execve(path, tokens[i], NULL);
                //Cюда заходить не должны!
                perror("execve failed");
                exit(EXIT_FAILURE);
            }

            pids[i] = pid;
        }

        for (int j = 0; j < token_count - 1; j++) {
            close(pipe_links[j][0]);
            close(pipe_links[j][1]);
        }

        for (int i = 0; i < MAX_SPLIT_AMOUNT; i++) {
            waitpid(pids[i], NULL, 0);
        }

        //printf("I waited for everyone!");
        write(STDOUT_FILENO, "I waited for everyone! \n", strlen("I waited for everyone! \n"));


        for (int i = 0; i < token_count; i++) {
            char **argv = tokens[i];
            free(argv);
        }

        free(tokens);
    }
}