// IOS proj2.c
// 17.4. 2023
// Autor: Marek Gergel, FIT
// Compiled: gcc 9.4.0

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

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

#define KEY_COUNTER 1
#define KEY_OFFICE_CLOSED 12

// shared memory
int counter_shmid;
int *counter;

int office_closed_shmid;
bool *office_closed;

typedef enum service
{
    LIST = 1,
    PACKAGE = 2,
    MONEY = 3
} service_t;

// process functions
void officer_p(unsigned long id, unsigned long pause);
void customer_p(unsigned long id, unsigned long wait);

// queue functions
bool queue_is_empty();

// helper functions
void append_to_file(const char *fmt, ...);
void free_resources();
void error_exit(int errcode, const char *fmt, ...);

/**
 * @brief Main process - creates customers and officers, controls office
 * @param argc argument count
 * @param argv argument values
 * @return int exit code
 */
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
    counter_shmid = shmget(KEY_COUNTER, sizeof(int), IPC_CREAT | 0666);
    counter = (int *)shmat(counter_shmid, NULL, 0);
    *counter = 0;

    office_closed_shmid = shmget(KEY_OFFICE_CLOSED, sizeof(bool), IPC_CREAT | 0666);
    office_closed = (bool *)shmat(office_closed_shmid, NULL, 0);
    *office_closed = false;

    // seed the random number generator with the current time
    srand(time(NULL));

    // customer generator
    for (unsigned long i = 0; i < param[0]; i++)
    {
        pid_t customer = fork();
        if (customer == 0)
        {
            customer_p(i, param[2]);
            exit(0);
        }
        else if (customer < 0)
        {
            error_exit(1, "Cannot create customer process\n");
        }
    }

    // officer generator
    for (unsigned long i = 0; i < param[1]; i++)
    {
        pid_t officer = fork();
        if (officer == 0)
        {
            officer_p(i, param[3]);
            exit(0);
        }
        else if (officer < 0)
        {
            error_exit(1, "Cannot create officer process\n");
        }
    }

    // sleep for <F/2, F> milliseconds
    int f_half = param[4] / 2;
    usleep((f_half == 0 ? 1 : (rand() % f_half + f_half)) * 1000);

    // close office
    *office_closed = true;
    append_to_file("closing\n");

    // wait for all officer and customer processes to finish
    while (wait(NULL) > 0)
        (void)0;

    // free resources
    free_resources();

    return 0;
}

// ----------------------- PROCESS FUNCTIONS ------------------------

/**
 * @brief Customer process
 * @param id customer id
 * @param wait max wait time milliseconds
 */
void customer_p(unsigned long id, unsigned long wait)
{
    append_to_file("Z %d: started\n", id);

    // seed the random number generator with the current time
    srand(time(NULL));

    // sleep for <0, TZ> milliseconds
    usleep(rand() % wait * 1000);

    if (!(*office_closed))
    {
        service_t service = (rand() % 3) + 1;

        append_to_file("Z %d: entering office for a service %d\n", id, service);

        // TODO wait for called by officer

        append_to_file("Z %d: called by office worker\n", id);

        // sleep for <0, 10>
        usleep(rand() % 11);
    }

    append_to_file("Z %d: going home\n", id);
}

/**
 * @brief Officer process
 * @param id officer id
 * @param pause max pause time milliseconds
 */
void officer_p(unsigned long id, unsigned long pause)
{
    append_to_file("U %d: started\n", id);

    // seed the random number generator with the current time
    srand(time(NULL));

    while (1)
    {

        // TODO check any customer is waiting
        if (!queue_is_empty())
        {
            (void)0;
        }
        else
        {
            if (!(*office_closed))
            {
                append_to_file("U %d: taking break\n", id);
                usleep(rand() % pause * 1000);
                append_to_file("U %d: break finished\n", id);
            }
            else
            {
                break;
            }
        }
    }

    append_to_file("U %d: going home\n", id);
}
// ------------------------ QUEUE FUNCTIONS -------------------------

bool queue_is_empty()
{
    return true;
}

// ------------------------ HELPER FUNCTIONS ------------------------

/**
 * @brief Append message to output file
 * @param fmt format string
 * @param ... arguments
 */
void append_to_file(const char *fmt, ...)
{
    // Open file for appending
    FILE *f = fopen(OUT_FILE, "a");
    if (f == NULL)
    {
        error_exit(1, "Cannot open file '%s'\n", OUT_FILE);
    }

    // Disable buffering
    // setbuf(f, NULL);

    // Print message
    va_list args;
    va_start(args, fmt);
    fprintf(f, "%d: ", ++(*counter));
    vfprintf(f, fmt, args);
    va_end(args);

    // Close file
    fclose(f);
}

/**
 * @brief Free allocated or shared memory
 */
void free_resources()
{
    shmdt(counter);                           // Detach from shared memory
    shmctl(counter_shmid, KEY_COUNTER, NULL); // Remove shared memory

    shmdt(office_closed);                                 // Detach from shared memory
    shmctl(office_closed_shmid, KEY_OFFICE_CLOSED, NULL); // Remove shared memory
}

/**
 * @brief Print error message and exit with error code
 * @param errcode error code
 * @param fmt format string
 * @param ... arguments
 */
void error_exit(int errcode, const char *fmt, ...)
{
    fflush(stderr);

    // Print error message
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Error : ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);

    // Free resources
    free_resources();

    // Exit with error code
    exit(errcode);
}