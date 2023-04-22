// proj2.c
// Řešení IOS, 5.5. 2020
// Autor: Marek Gergel, FIT
// Přeloženo: gcc 7.5.0
// 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <ctype.h>

#define PARAMS 5    //number of values in arguments , {PI, IG, JG, IT, JT}

void gen_imm(unsigned long count, unsigned long delay);
void error_exit(int errcode, const char *fmt, ...);

int main (int argc, char* argv[]) {

    //invalid number of arguments
    if (argc != PARAMS + 1) error_exit(1,"Invalid number of argumets\n");

    //variables to convert and hold arguments
    unsigned long param[PARAMS];
    char* ptr;

    //convert arguments into ulong value
    for (int i = 0; i < PARAMS; i++) {
        param[i] = strtoul(argv[i+1], &ptr ,0);
        if (strcmp(ptr,"") != 0) {
            error_exit(1,"Argument %d. '%s' is not a number\n",i+1,argv[i+1]);
        } else if (i > 0 && (param[i] > 2000 || argv[i+1][0] == '-')) {
            error_exit(1,"Argument %d. '%s' must be in range 0 and 2000\n",i+1,argv[i+1]);
        } else if (i == 0 && (param[i] == 0 || argv[i+1][0] == '-')) {
            error_exit(1,"Argument %d. '%s' must be bigger than 0\n",i+1,argv[i+1]);
        }
    }

    //
    pid_t just = fork();
    if (just == 0) gen_imm(param[0],param[1]);



    return 0;
}

void gen_imm(unsigned long count, unsigned long delay) {

    for (unsigned long i = 0; i < count; i++) {

        sleep(rand() % delay);


    }


}

void error_exit(int errcode, const char *fmt, ...) {
    va_list args;
    va_start (args, fmt);
    fprintf(stderr, "Error : ");
    vfprintf(stderr, fmt, args);
    va_end (args);
    exit(errcode);
}