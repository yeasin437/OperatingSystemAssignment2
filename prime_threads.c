#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

/* 
   This structure is used to give each thread its own data.
   start = first number in that thread's range
   end   = last number in that thread's range
   count = number of primes found by that thread
*/
typedef struct {
    int start;
    int end;
    int count;
} ThreadData;

/* 
   Checks if a number is prime.
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
   This function is executed by each thread.

   Each thread receives a pointer to its own ThreadData structure.
   It checks all numbers from start to end and counts how many are prime.
   When finished, it saves the result in data->count.
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
   Returns current time in milliseconds.
   Used to measure execution time.
*/
double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

/*
   Single-threaded version.
   The main thread alone checks the full range.
*/
void single_thread(int max) {
    double start_time = get_time_ms();
    int total = 0;

    for (int i = 1; i <= max; i++) {
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
   Multi-threaded version.

   The full range is divided into smaller parts.
   Each part is assigned to one thread.
   All threads run the same function, count_primes(),
   but each thread works on a different range.
*/
void multi_thread(int num_threads, int max) {
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    int step = max / num_threads;
    double start_time = get_time_ms();

    /*
       THREAD CREATION LOGIC:

       This loop creates all worker threads.
       For each thread:
       1. assign a start value
       2. assign an end value
       3. set count to 0
       4. call pthread_create()

       pthread_create() starts a new thread.
       That new thread begins running count_primes()
       and receives &data[i] as its argument.
    */
    for (int i = 0; i < num_threads; i++) {
        data[i].start = i * step + 1;

        /*
           The last thread takes the remaining numbers too.
           This makes sure the full range up to max is covered.
        */
        if (i == num_threads - 1)
            data[i].end = max;
        else
            data[i].end = (i + 1) * step;

        data[i].count = 0;

        if (pthread_create(&threads[i], NULL, count_primes, &data[i]) != 0) {
            printf("Error creating thread %d\n", i);
            return;
        }
    }

    int total = 0;

    /*
       THREAD JOINING LOGIC:

       pthread_join() makes the main thread wait until
       each worker thread finishes its work.

       This is necessary because the main thread should not
       print the final answer before all threads complete.

       After a thread finishes, its prime count is stored in data[i].count.
       The main thread adds that partial result to total.
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
    int max = 500000;

    /* get number of logical processors available */
    int max_threads = sysconf(_SC_NPROCESSORS_ONLN);

    printf("Prime Number Finder (1 to %d)\n", max);
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
        single_thread(max);
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

        multi_thread(num_threads, max);
    }
    else {
        printf("Invalid choice.\n");
    }

    return 0;
}
