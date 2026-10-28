/*
 * =====================================================================================
 * Program 07 (Creative Extension): omp_lock.c
 * Description: Demonstrates advanced OpenMP explicit lock primitives (omp_lock_t)
 *              including blocking locks (omp_set_lock) and non-blocking lock polling
 *              (omp_test_lock) with fallback workloads.
 * Course:      Parallel and GPU Computing (PGC)
 * Author:      Soumya Surpur (USN: 01FE24BCI121)
 * Institution: KLE Technological University
 * =====================================================================================
 * Compilation: gcc omp_lock.c -fopenmp -o omp_lock
 * Execution:   ./omp_lock
 * =====================================================================================
 * Key Theoretical & Architectural Insights:
 * 1. Explicit Locking Control: Unlike '#pragma omp critical' which is tied to compiler
 *    code blocks, 'omp_lock_t' allows fine-grained, dynamic programmatic control over
 *    lock initialization, acquisition, releasing, and destruction.
 * 2. Non-Blocking Acquisition: 'omp_test_lock(&lock)' attempts to acquire the lock.
 *    If busy, instead of stalling the thread in an idle wait state, it immediately
 *    returns false, enabling the thread to execute auxiliary work (latency hiding).
 * 3. Lock API Lifecycle:
 *      omp_init_lock()    -> Initializes internal lock data structure.
 *      omp_set_lock()     -> Blocks until the lock becomes available.
 *      omp_test_lock()    -> Tries non-blocking acquisition (returns 1 on success, 0 on busy).
 *      omp_unset_lock()   -> Releases the lock for other threads.
 *      omp_destroy_lock() -> Deallocates lock resources.
 * =====================================================================================
 */

#include <stdio.h>
#include <omp.h>
#include <unistd.h>

#define N 5000000L

int main(void) {
    long counter = 0;
    long fallback_work_done = 0;
    omp_lock_t lock;

    printf("=====================================================\n");
    printf(" Creative Program: OpenMP Explicit Lock (omp_lock_t)\n");
    printf(" Feature: Blocking & Non-Blocking (omp_test_lock)\n");
    printf(" Workload: %ld Increments\n", N);
    printf("=====================================================\n");

    // Initialize the lock
    omp_init_lock(&lock);

    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        omp_set_lock(&lock);   // Blocking lock acquisition
        counter++;
        omp_unset_lock(&lock); // Release lock
    }

    double elapsed_time = omp_get_wtime() - start_time;

    printf("[1] Blocking Lock (omp_set_lock):\n");
    printf("    Final Counter   : %ld / %ld\n", counter, N);
    printf("    Execution Time  : %.6f seconds\n", elapsed_time);
    printf("    Status          : %s\n\n", (counter == N) ? "CORRECT" : "INCORRECT");

    // Reset counter for Non-Blocking Test
    counter = 0;
    start_time = omp_get_wtime();

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        long local_fallback = 0;

        #pragma omp for
        for (long i = 0; i < 1000000L; i++) {
            if (omp_test_lock(&lock)) {
                // Successfully acquired lock without blocking
                counter++;
                omp_unset_lock(&lock);
            } else {
                // Lock was busy! Do useful alternate computation instead of waiting
                local_fallback++;
            }
        }

        #pragma omp atomic
        fallback_work_done += local_fallback;
    }

    elapsed_time = omp_get_wtime() - start_time;

    printf("[2] Non-Blocking Lock Polling (omp_test_lock):\n");
    printf("    Increments Acquired : %ld\n", counter);
    printf("    Fallback Cycles Done: %ld\n", fallback_work_done);
    printf("    Execution Time      : %.6f seconds\n", elapsed_time);
    printf("    Insight             : Avoids thread stalling when lock is contended!\n");
    printf("=====================================================\n");

    // Destroy the lock
    omp_destroy_lock(&lock);
    return 0;
}
