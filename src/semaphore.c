/*
 * =====================================================================================
 * Program 05: semaphore.c
 * Description: Implements counting/binary semaphore synchronization using POSIX
 *              Threads (Pthreads) and <semaphore.h> to prevent data corruption.
 * Course:      Parallel and GPU Computing (PGC)
 * Author:      Soumya Surpur (USN: 01FE24BCI121)
 * Institution: KLE Technological University
 * =====================================================================================
 * Compilation: gcc semaphore.c -pthread -o semaphore
 * Execution:   ./semaphore
 * =====================================================================================
 * Observed Lab Outputs (from output_03_programs_execution.jpeg):
 *   Counter = 200000 (Expected: 200000 -> 100000 per thread x 2 threads)
 * =====================================================================================
 * Key Theoretical Insights:
 * 1. Counter-Based Kernel Primitive: A semaphore maintains an integer state initialized
 *    via 'sem_init(&sem, pshared, value)'. Setting value = 1 makes it a binary semaphore (mutex).
 * 2. Atomic Decrement and Wait: 'sem_wait(&sem)' atomically decrements the counter. If the
 *    counter is <= 0, the calling thread is suspended into the OS wait queue.
 * 3. Signal and Wakeup: 'sem_post(&sem)' atomically increments the counter and unblocks
 *    a waiting thread, if any.
 * 4. Generality vs. Overhead: Counting semaphores can manage access to pools of N identical
 *    resources (unlike binary mutexes). However, transitioning between user space and kernel
 *    space (via Linux futex system calls) incurs higher latency than user-space hardware atomics.
 * =====================================================================================
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define WORKLOAD_PER_THREAD 100000L

sem_t semaphore;
long counter = 0;

void* work(void* arg) {
    long thread_id = (long)arg;
    for (long i = 0; i < WORKLOAD_PER_THREAD; i++) {
        sem_wait(&semaphore); // Decrement semaphore; blocks if zero
        counter++;            // Protected critical section
        sem_post(&semaphore); // Increment semaphore; signals waiting thread
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    printf("=====================================================\n");
    printf(" Experiment: POSIX Binary Semaphore Synchronization\n");
    printf(" Library:    <semaphore.h> + <pthread.h>\n");
    printf(" Workload:   2 Threads x %ld Increments = %ld Total\n", 
           WORKLOAD_PER_THREAD, 2 * WORKLOAD_PER_THREAD);
    printf("=====================================================\n");

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Initialize semaphore: pshared=0 (threads within process), initial value=1 (binary mutex)
    if (sem_init(&semaphore, 0, 1) != 0) {
        perror("Failed to initialize semaphore");
        return 1;
    }

    // Spawn 2 worker threads
    pthread_create(&t1, NULL, work, (void*)1L);
    pthread_create(&t2, NULL, work, (void*)2L);

    // Wait for worker threads to complete execution
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Final Counter Value : %ld\n", counter);
    printf("Expected Value      : %ld\n", 2 * WORKLOAD_PER_THREAD);
    printf("Execution Time      : %.6f seconds\n", elapsed_time);
    printf("Status              : %s\n", 
           (counter == 2 * WORKLOAD_PER_THREAD) ? "CORRECT" : "INCORRECT");
    printf("Observation         : Kernel-level semaphore guarantees strict mutual exclusion.\n");
    printf("=====================================================\n");

    // Clean up semaphore resource
    sem_destroy(&semaphore);
    return 0;
}
