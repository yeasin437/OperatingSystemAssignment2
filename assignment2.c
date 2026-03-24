#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define MAX_NUMBER 500000

typedef struct
{
    int start;
    int end;
    int prime_count;
} ThreadData;

/* 
   Checks whether a number is prime.
   Returns 1 if prime, 0 otherwise.
*/
int is_prime(int n)
{
    int i;

    if (n < 2)
    {
        return 0;
    }

    if (n == 2)
    {
        return 1;
    }

    if (n % 2 == 0)
    {
        return 0;
    }

    for (i = 3; i * i <= n; i += 2)
    {
        if (n % i == 0)
        {
            return 0;
        }
    }

    return 1;
}

/*
   This is the function each thread runs.
   Each thread checks prime numbers only in its assigned range.
*/
void *count_primes_in_range(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int count = 0;
    int i;

    for (i = data->start; i <= data->end; i++)
    {
        if (is_prime(i))
        {
            count++;
        }
    }

    data->prime_count = count;
    pthread_exit(NULL);
}

/*
   Runs the prime search using exactly 1 thread.
*/
void run_single_threaded()
{
    ThreadData data;
    pthread_t thread;
    struct timespec start_time, end_time;
    long seconds, nanoseconds;
    double elapsed_ms;

    data.start = 1;
    data.end = MAX_NUMBER;
    data.prime_count = 0;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    /*
       Create one thread to process the full range.
    */
    if (pthread_create(&thread, NULL, count_primes_in_range, &data) != 0)
    {
        printf("Error creating thread.\n");
        return;
    }

    /*
       Wait for the thread to finish before continuing.
       This is called joining the thread.
    */
    if (pthread_join(thread, NULL) != 0)
    {
        printf("Error joining thread.\n");
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    seconds = end_time.tv_sec - start_time.tv_sec;
    nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
    elapsed_ms = (seconds * 1000.0) + (nanoseconds / 1000000.0);

    printf("\n----- Single-Threaded Mode -----\n");
    printf("Range checked: 1 to %d\n", MAX_NUMBER);
    printf("Total prime numbers found: %d\n", data.prime_count);
    printf("Execution time: %.3f ms\n", elapsed_ms);
}

/*
   Runs the prime search using multiple threads.
*/
void run_multi_threaded(int num_threads)
{
    pthread_t *threads;
    ThreadData *thread_data;
    struct timespec start_time, end_time;
    long seconds, nanoseconds;
    double elapsed_ms;
    int range_size;
    int start;
    int end;
    int i;
    int total_primes = 0;

    if (num_threads < 1)
    {
        printf("Invalid number of threads.\n");
        return;
    }

    threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    thread_data = (ThreadData *)malloc(num_threads * sizeof(ThreadData));

    if (threads == NULL || thread_data == NULL)
    {
        printf("Memory allocation failed.\n");
        free(threads);
        free(thread_data);
        return;
    }

    range_size = MAX_NUMBER / num_threads;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    /*
       Create all threads.
       Each thread gets a different part of the range.
    */
    for (i = 0; i < num_threads; i++)
    {
        start = i * range_size + 1;

        if (i == num_threads - 1)
        {
            end = MAX_NUMBER;
        }
        else
        {
            end = (i + 1) * range_size;
        }

        thread_data[i].start = start;
        thread_data[i].end = end;
        thread_data[i].prime_count = 0;

        if (pthread_create(&threads[i], NULL, count_primes_in_range, &thread_data[i]) != 0)
        {
            printf("Error creating thread %d.\n", i + 1);
            free(threads);
            free(thread_data);
            return;
        }
    }

    /*
       Join all threads.
       This makes the main program wait until every thread is finished.
    */
    for (i = 0; i < num_threads; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            printf("Error joining thread %d.\n", i + 1);
            free(threads);
            free(thread_data);
            return;
        }

        total_primes += thread_data[i].prime_count;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    seconds = end_time.tv_sec - start_time.tv_sec;
    nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
    elapsed_ms = (seconds * 1000.0) + (nanoseconds / 1000000.0);

    printf("\n----- Multi-Threaded Mode -----\n");
    printf("Range checked: 1 to %d\n", MAX_NUMBER);
    printf("Threads used: %d\n", num_threads);
    printf("Total prime numbers found: %d\n", total_primes);
    printf("Execution time: %.3f ms\n", elapsed_ms);

    free(threads);
    free(thread_data);
}

int main()
{
    int choice;
    int num_threads;
    long logical_processors;

    printf("Prime Number Finder (1 to %d)\n", MAX_NUMBER);
    printf("1. Single-threaded mode\n");
    printf("2. Multi-threaded mode\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 1)
    {
        run_single_threaded();
    }
    else if (choice == 2)
    {
        logical_processors = sysconf(_SC_NPROCESSORS_ONLN);

        printf("Logical processors available on this machine: %ld\n", logical_processors);
        printf("Enter number of threads to use: ");
        scanf("%d", &num_threads);

        if (num_threads < 1)
        {
            printf("Number of threads must be at least 1.\n");
            return 1;
        }

        run_multi_threaded(num_threads);
    }
    else
    {
        printf("Invalid choice.\n");
    }

    return 0;
}
