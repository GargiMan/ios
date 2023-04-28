// IOS proj2.c
// 17.4. 2023
// Autor: Marek Gergel, FIT
// Compiled: gcc 9.4.0

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <ctype.h>

#define PARAMS 5 // number of required values in arguments {NZ, NU, TZ, TU, F}
#define OUT_FILE "proj2.out"

// shared memory
int shmid;
int *shared_var;

void gen_officer(unsigned long count, unsigned long pause);
void gen_customer(unsigned long count, unsigned long delay);
void append_to_file(const char *fmt, ...);
void free_resources();
void error_exit(int errcode, const char *fmt, ...);

int main(int argc, char *argv[])
{

    // invalid number of arguments
    if (argc - 1 != PARAMS)
    {
        error_exit(1, "Invalid number of argumets\n");
    }

    // create/clean output file
    FILE *f = fopen(OUT_FILE, "w");
    if (f == NULL)
    {
        error_exit(2, "Cannot open file '%s'\n", OUT_FILE);
    }
    fclose(f);

    // convert arguments into ulong value
    unsigned long param[PARAMS];
    for (int i = 0; i < PARAMS; i++)
    {
        char *ptr;
        param[i] = strtoul(argv[i + 1], &ptr, 0);
        if (strcmp(ptr, "") != 0)
        {
            error_exit(1, "Argument %d. '%s' is not a number\n", i + 1, argv[i + 1]);
        }
        else if ((i == 0 || i == 1) && (param[i] == 0 || argv[i + 1][0] == '-'))
        {
            error_exit(1, "Argument %d. '%s' must be bigger than 0\n", i + 1, argv[i + 1]);
        }
        else if ((i == 2 || i == 4) && (param[i] > 10000 || argv[i + 1][0] == '-'))
        {
            error_exit(1, "Argument %d. '%s' must be in range 0 and 10000\n", i + 1, argv[i + 1]);
        }
        else if ((i == 3) && (param[i] > 100 || argv[i + 1][0] == '-'))
        {
            error_exit(1, "Argument %d. '%s' must be in range 0 and 100\n", i + 1, argv[i + 1]);
        }
    }

    // create shared memory
    shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    shared_var = (int *)shmat(shmid, NULL, 0);
    *shared_var = 0;

    // customer generator
    pid_t customer = fork();
    if (customer == 0)
    {
        gen_customer(param[0], param[2]);
    }
    else
    {
        // officer generator
        pid_t officer = fork();
        if (officer == 0)
        {
            gen_officer(param[1], param[3]);
        }
        else
        {
            append_to_file("closing\n");
        }
    }

    sleep(10);

    free_resources();

    return 0;
}

void gen_customer(unsigned long count, unsigned long delay)
{

    for (unsigned long i = 0; i < count; i++)
    {
        append_to_file("Z %d: started\n", i);
        usleep(rand() % delay * 1000);
        append_to_file("Z %d: finished\n");
    }
}

void gen_officer(unsigned long count, unsigned long pause)
{

    for (unsigned long i = 0; i < count; i++)
    {
        append_to_file("Z %d: started\n", i);
        usleep(rand() % (pause * 1000));
        append_to_file("Z %d: finished\n");
    }
}

void append_to_file(const char *fmt, ...)
{

    FILE *f = fopen(OUT_FILE, "a");
    if (f == NULL)
    {
        error_exit(1, "Cannot open file '%s'\n", OUT_FILE);
    }

    *shared_var += 1;
    fprintf(f, "%d: ", *shared_var);

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fflush(f);

    fclose(f);
}

void free_resources()
{
    shmdt(shared_var);             // Detach from shared memory
    shmctl(shmid, IPC_RMID, NULL); // Remove shared memory
}

void error_exit(int errcode, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Error : ");
    vfprintf(stderr, fmt, args);
    va_end(args);

    free_resources();

    exit(errcode);
}