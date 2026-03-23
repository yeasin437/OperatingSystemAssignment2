#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#define MAX 500000

// structure to store data for each thread
typedef struct {
    int start;
    int end;
    int count;
} ThreadData;

// function to check if a number is prime
int is_prime(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;

    int limit = (int)sqrt(n);
    for (int i = 3; i <= limit; i += 2) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

// thread function: count primes in assigned range
void* count_primes(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int count = 0;

    for (int i = data->start; i <= data->end; i++) {
        if (is_prime(i)) {
            count++;
        }
    }

    data->count = count; // store result
    pthread_exit(NULL);
}

// get current time in milliseconds
double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

// single-thread version
void single_thread() {
    double start = get_time_ms();

    int total = 0;
    for (int i = 1; i <= MAX; i++) {
        if (is_prime(i)) {
            total++;
        }
    }

    double end = get_time_ms();

    printf("\nSingle-threaded:\n");
    printf("Total primes: %d\n", total);
    printf("Execution time: %.2f ms\n", end - start);
}

// multi-thread version
void multi_thread(int num_threads) {
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    int step = MAX / num_threads; // divide range
    double start = get_time_ms();

    // create threads
    for (int i = 0; i < num_threads; i++) {
        data[i].start = i * step + 1;

        // last thread takes remaining part
        if (i == num_threads - 1)
            data[i].end = MAX;
        else
            data[i].end = (i + 1) * step;

        data[i].count = 0;

        pthread_create(&threads[i], NULL, count_primes, &data[i]);
    }

    int total = 0;

    // wait for all threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total += data[i].count;
    }

    double end = get_time_ms();

    printf("\nMulti-threaded (%d threads):\n", num_threads);
    printf("Total primes: %d\n", total);
    printf("Execution time: %.2f ms\n", end - start);
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
    scanf("%d", &choice);

    if (choice == 1) {
        single_thread();
    }
    else if (choice == 2) {
        printf("Enter number of threads (1 to %d): ", max_threads);
        scanf("%d", &num_threads);

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
