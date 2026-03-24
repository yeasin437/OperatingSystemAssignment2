#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#define MAX 500000

/* 
   This structure is used to give each thread its own range.
   start = first number to check
   end   = last number to check
   count = how many primes this thread found
*/
typedef struct {
    int start;
    int end;
    int count;
} ThreadData;

/* 
   Check if a number is prime.
   Returns 1 if prime, 0 if not prime.
*/
int is_prime(int n) {
    if (n < 2)
        return 0;

    if (n == 2)
        return 1;

    if (n % 2 == 0)
        return 0;

    int limit = (int)sqrt(n);

    for (int i = 3; i <= limit; i += 2) {
        if (n % i == 0)
            return 0;
    }

    return 1;
}

/*
   This is the function each thread runs.

   The thread receives a pointer to its own ThreadData.
   It checks all numbers in its assigned range and counts primes.
   When finished, it stores the result in data->count.
*/
void* count_primes(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int local_count = 0;

    for (int i = data->start; i <= data->end; i++) {
        if (is_prime(i)) {
            local_count++;
        }
    }

    data->count = local_count;
    pthread_exit(NULL);
}

/*
   Get current time in milliseconds.
   This is used to measure execution time.
*/
double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

/*
   Single-threaded version:
   One thread (the main thread) does all the work.
*/
void single_thread() {
    double start_time = get_time_ms();

    int total = 0;

    for (int i = 1; i <= MAX; i++) {
        if (is_prime(i)) {
            total++;
        }
    }

    double end_time = get_time_ms();

    printf("\nSingle-threaded:\n");
    printf("Total primes: %d\n", total);
    printf("Execution time: %.2f ms\n", end_time - start_time);
}

/*
   Multi-threaded version:
   The range 1 to MAX is divided into smaller parts.
   Each thread gets one part and counts primes in that part.
*/
void multi_thread(int num_threads) {
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    int step = MAX / num_threads;
    double start_time = get_time_ms();

    /*
       Create threads.

       For each thread:
       - set its start and end range
       - set count to 0
       - call pthread_create() to start the thread

       pthread_create() starts a new thread and makes it run
       the count_primes function.
    */
    for (int i = 0; i < num_threads; i++) {
        data[i].start = i * step + 1;

        /* 
           The last thread takes the remaining numbers too,
           so the full range up to MAX is covered.
        */
        if (i == num_threads - 1) {
            data[i].end = MAX;
        } else {
            data[i].end = (i + 1) * step;
        }

        data[i].count = 0;

        if (pthread_create(&threads[i], NULL, count_primes, &data[i]) != 0) {
            printf("Error creating thread %d\n", i);
            return;
        }
    }

    int total = 0;

    /*
       Join threads.

       pthread_join() makes the main thread wait until
       each worker thread finishes.

       This is important because we should not add the results
       until all threads are done with their work.
    */
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            printf("Error joining thread %d\n", i);
            return;
        }

        total += data[i].count;
    }

    double end_time = get_time_ms();

    printf("\nMulti-threaded (%d threads):\n", num_threads);
    printf("Total primes: %d\n", total);
    printf("Execution time: %.2f ms\n", end_time - start_time);
}

int main() {
    int choice;
    int num_threads;
    int max_threads = sysconf(_SC_NPROCESSORS_ONLN);

    printf("Prime Number Finder (1 to %d)\n", MAX);
    printf("Logical processors: %d\n", max_threads);

    printf("\nChoose mode:\n");
    printf("1. Single-threaded\n");
    printf("2. Multi-threaded\n");
    printf("Enter choice: ");

    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        return 1;
    }

    if (choice == 1) {
        single_thread();
    }
    else if (choice == 2) {
        printf("Enter number of threads (1 to %d): ", max_threads);

        if (scanf("%d", &num_threads) != 1) {
            printf("Invalid input.\n");
            return 1;
        }

        if (num_threads < 1 || num_threads > max_threads) {
            printf("Invalid number of threads.\n");
            return 1;
        }

        multi_thread(num_threads);
    }
    else {
        printf("Invalid choice.\n");
    }

    return 0;
}
