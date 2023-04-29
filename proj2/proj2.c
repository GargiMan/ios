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
#include <semaphore.h>
#include <ctype.h>

#define PARAMS 5   // number of required values in arguments {NZ, NU, TZ, TU, F}
#define SERVICES 3 // number of services
#define OUT_FILE "proj2.out"

#define KEY_COUNTER 1234
#define KEY_OFFICE_CLOSED 12345
#define KEY_QUEUE 123456

// services
typedef enum service
{
    LIST = 0,
    PACKAGE = 1,
    MONEY = 2
} service_t;

// shared memory
int counter_shmid;
int *counter;

int office_closed_shmid;
bool *office_closed;

int queue_shmid;
int *queue;

// semaphores
sem_t *sem_called_list;
sem_t *sem_called_package;
sem_t *sem_called_money;
sem_t *sem_output;

// output file
FILE *f = NULL;

// process functions
void officer_p(unsigned long id, unsigned long pause);
void customer_p(unsigned long id, unsigned long wait);

// semaphores functions
void sems_init();
void sems_destroy();

// helper functions
void append_to_file(const char *fmt, ...);
void free_resources();
void error_exit(bool free, int errcode, const char *fmt, ...);

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
        error_exit(false, 1, "Invalid number of argumets\n");
    }

    // open output file
    if ((f = fopen(OUT_FILE, "w")) == NULL)
    {
        error_exit(false, 2, "Cannot open file '%s'\n", OUT_FILE);
    }

    // convert arguments into ulong value
    unsigned long param[PARAMS];
    for (int i = 0; i < PARAMS; i++)
    {
        char *ptr;
        param[i] = strtoul(argv[i + 1], &ptr, 0);
        if (strcmp(ptr, "") != 0)
        {
            error_exit(false, 1, "Argument %d. '%s' is not a number\n", i + 1, argv[i + 1]);
        }
        else if ((i == 0 || i == 1) && (param[i] == 0 || argv[i + 1][0] == '-'))
        {
            error_exit(false, 1, "Argument %d. '%s' must be bigger than 0\n", i + 1, argv[i + 1]);
        }
        else if ((i == 2 || i == 4) && (param[i] > 10000 || argv[i + 1][0] == '-'))
        {
            error_exit(false, 1, "Argument %d. '%s' must be in range 0 and 10000\n", i + 1, argv[i + 1]);
        }
        else if ((i == 3) && (param[i] > 100 || argv[i + 1][0] == '-'))
        {
            error_exit(false, 1, "Argument %d. '%s' must be in range 0 and 100\n", i + 1, argv[i + 1]);
        }
    }

    // create shared memory
    counter_shmid = shmget(KEY_COUNTER, sizeof(int), IPC_CREAT | 0666);
    counter = (int *)shmat(counter_shmid, NULL, 0);
    *counter = 0;

    office_closed_shmid = shmget(KEY_OFFICE_CLOSED, sizeof(bool), IPC_CREAT | 0666);
    office_closed = (bool *)shmat(office_closed_shmid, NULL, 0);
    *office_closed = false;

    queue_shmid = shmget(KEY_QUEUE, sizeof(int) * SERVICES, IPC_CREAT | 0666);
    queue = (int *)shmat(queue_shmid, NULL, 0);
    queue[LIST] = 0;
    queue[PACKAGE] = 0;
    queue[MONEY] = 0;

    // create semaphores
    sems_init();

    // post semaphores
    sem_post(sem_output);

    // seed the random number generator with the current time
    srand(getpid() * time(NULL));

    // customer generator
    for (unsigned long i = 0; i < param[0]; i++)
    {
        pid_t customer = fork();
        if (customer == 0)
        {
            customer_p(i, param[2]);
        }
        else if (customer < 0)
        {
            error_exit(true, 1, "Cannot create customer process\n");
        }
    }

    // officer generator
    for (unsigned long i = 0; i < param[1]; i++)
    {
        pid_t officer = fork();
        if (officer == 0)
        {
            officer_p(i, param[3]);
        }
        else if (officer < 0)
        {
            error_exit(true, 1, "Cannot create officer process\n");
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
    srand(getpid() * time(NULL));

    // sleep for <0, TZ> milliseconds
    usleep(rand() % wait * 1000);

    if (!(*office_closed))
    {
        service_t service = rand() % SERVICES;

        append_to_file("Z %d: entering office for a service %d\n", id, service + 1);

        switch (service)
        {
        case LIST:
            queue[LIST]++;
            // printf("LIST: %d\n", queue[LIST]);
            sem_wait(sem_called_list);
            break;
        case PACKAGE:
            queue[PACKAGE]++;
            // printf("PACKAGE: %d\n", queue[PACKAGE]);
            sem_wait(sem_called_package);
            break;
        case MONEY:
            queue[MONEY]++;
            // printf("MONEY: %d\n", queue[MONEY]);
            sem_wait(sem_called_money);
            break;
        }

        append_to_file("Z %d: called by office worker\n", id);

        // sleep for <0, 10>
        usleep(rand() % 11);
    }

    append_to_file("Z %d: going home\n", id);

    exit(0);
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
    srand(getpid() * time(NULL));

    while (1)
    {

        if (queue[LIST] + queue[PACKAGE] + queue[MONEY] != 0)
        {
            service_t service, service1, service2, service3;

            service1 = rand() % SERVICES;
            service = service1;

            if (queue[service] == 0)
            {
                do
                {
                    service2 = rand() % SERVICES;
                } while (service2 == service1);
                service = service2;

                if (queue[service] == 0)
                {
                    do
                    {
                        service3 = rand() % SERVICES;
                    } while (service3 == service1 || service3 == service2);
                    service = service3;

                    if (queue[service] == 0)
                    {
                        continue;
                    }
                }
            }

            switch (service)
            {
            case LIST:
                queue[LIST]--;
                // printf("LIST: %d\n", queue[LIST]);
                sem_post(sem_called_list);
                break;
            case PACKAGE:
                queue[PACKAGE]--;
                // printf("PACKAGE: %d\n", queue[PACKAGE]);
                sem_post(sem_called_package);
                break;
            case MONEY:
                queue[MONEY]--;
                // printf("MONEY: %d\n", queue[MONEY]);
                sem_post(sem_called_money);
                break;
            }

            append_to_file("U %d: serving a service of type %d\n", id, service + 1);

            // sleep for <0, 10>
            usleep(rand() % 11);

            append_to_file("U %d: service finished\n", id);
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

    exit(0);
}

// ---------------------- SEMAPHORE FUNCTIONS -----------------------

void sems_init()
{
    sem_called_list = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_called_package = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_called_money = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_output = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (sem_init(sem_called_list, 1, 0) == -1 || sem_init(sem_called_package, 1, 0) == -1 || sem_init(sem_called_money, 1, 0) == -1 || sem_init(sem_output, 1, 0) == -1)
    {
        error_exit(true, 1, "Cannot initialize semaphore\n");
    }
}

void sems_destroy()
{
    int error = 0;

    error |= sem_destroy(sem_called_list) | sem_destroy(sem_called_package) | sem_destroy(sem_called_money) | sem_destroy(sem_output);
    error |= munmap(sem_called_list, sizeof(sem_t)) | munmap(sem_called_package, sizeof(sem_t)) | munmap(sem_called_money, sizeof(sem_t)) | munmap(sem_output, sizeof(sem_t));

    if (error != 0)
    {
        error_exit(false, 1, "Cannot destroy semaphore\n");
    }
}

// ------------------------ HELPER FUNCTIONS ------------------------

/**
 * @brief Append message to output file
 * @param fmt format string
 * @param ... arguments
 */
void append_to_file(const char *fmt, ...)
{
    sem_wait(sem_output);

    va_list args;
    va_start(args, fmt);
    fprintf(f, "%d: ", ++(*counter));
    vfprintf(f, fmt, args);
    va_end(args);
    fflush(f);

    sem_post(sem_output);
}

/**
 * @brief Free allocated or shared memory
 */
void free_resources()
{
    // Close output file
    fclose(f);

    // Detach and remove shared memory
    shmdt(counter);
    shmctl(counter_shmid, KEY_COUNTER, NULL);
    shmdt(office_closed);
    shmctl(office_closed_shmid, KEY_OFFICE_CLOSED, NULL);
    shmdt(queue);
    shmctl(queue_shmid, KEY_QUEUE, NULL);

    // Destroy semaphores
    sems_destroy();
}

/**
 * @brief Print error message and exit with error code
 * @param free boolean value indicating whether to free resources
 * @param errcode error code
 * @param fmt format string
 * @param ... arguments
 */
void error_exit(bool free, int errcode, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Error : ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);

    if (free)
    {
        free_resources();
    }

    exit(errcode);
}