void multi_thread(int num_threads) {
    pthread_t threads[num_threads];
    ThreadData data[num_threads];

    // Divide the full range into approximately equal parts.
    // Each thread will process one subrange independently.
    int step = MAX / num_threads;
    double start = get_time_ms();

    // Create worker threads.
    // Each thread receives its own ThreadData structure containing:
    // 1. start of range
    // 2. end of range
    // 3. place to store its partial prime count
    for (int i = 0; i < num_threads; i++) {
        data[i].start = i * step + 1;

        // The last thread handles any remaining numbers
        // so that the full range up to MAX is covered.
        if (i == num_threads - 1)
            data[i].end = MAX;
        else
            data[i].end = (i + 1) * step;

        data[i].count = 0;

        // pthread_create starts a new thread.
        // That thread begins execution in count_primes()
        // and uses &data[i] as its argument.
        pthread_create(&threads[i], NULL, count_primes, &data[i]);
    }

    int total = 0;

    // pthread_join makes the main thread wait for each worker thread
    // to finish before continuing.
    // This ensures all partial results are ready before summing them.
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total += data[i].count;
    }

    double end = get_time_ms();

    printf("\nMulti-threaded (%d threads):\n", num_threads);
    printf("Total primes: %d\n", total);
    printf("Execution time: %.2f ms\n", end - start);
}
