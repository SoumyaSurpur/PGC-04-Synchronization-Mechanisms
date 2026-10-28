/*
 * =====================================================================================
 * Program 02: critical.c
 * Description: Implements mutual exclusion using the OpenMP '#pragma omp critical'
 *              directive to protect shared counter updates from concurrent access.
 * Course:      Parallel and GPU Computing (PGC)
 * Author:      Soumya Surpur (USN: 01FE24BCI121)
 * Institution: KLE Technological University
 * =====================================================================================
 * Compilation: gcc critical.c -fopenmp -o critical
 * Execution:   ./critical
 *              export OMP_NUM_THREADS=8; ./critical
 * =====================================================================================
 * Observed Lab Outputs (from terminal outputs):
 *   - Default / 4 Threads: Counter = 10000000 (100% Correct)
 *   - 8 Threads:           Counter = 10000000 (100% Correct)
 *   - Measured Timings:    1 Thread: 0.0904s | 2 Threads: 0.3072s | 4 Threads: 0.6614s | 8 Threads: 0.6665s
 * =====================================================================================
 * Key Theoretical Insights:
 * 1. Mutual Exclusion Guarantee: Only one thread at any instant is permitted inside
 *    the structured block governed by '#pragma omp critical'. Other threads wait in queue.
 * 2. Complete Serialization: While correctness is guaranteed (no lost updates), all
 *    concurrency is effectively neutralized within the critical section.
 * 3. Lock Contention Overhead: As thread count increases from 1 to 4, execution time jumps
 *    from 0.0904s to 0.6614s (~7.3x slowdown). Adding threads degrades performance
 *    due to thread queuing, cache line invalidations, and context arbitration.
 * =====================================================================================
 */

#include <stdio.h>
#include <omp.h>

#define N 10000000L

int main(void) {
    long counter = 0;

    printf("=====================================================\n");
    printf(" Experiment: Critical Section Synchronization\n");
    printf(" Directive:  #pragma omp critical\n");
    printf(" Workload:   %ld Shared Increments\n", N);
    printf("=====================================================\n");

    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        #pragma omp critical
        {
            counter++; // Serialized access: only one thread executes at a time
        }
    }

    double elapsed_time = omp_get_wtime() - start_time;

    printf("Final Counter Value : %ld\n", counter);
    printf("Expected Value      : %ld\n", N);
    printf("Execution Time      : %.6f seconds\n", elapsed_time);
    printf("Status              : %s\n", (counter == N) ? "CORRECT" : "INCORRECT");
    printf("Observation         : Perfectly synchronized, but serializes parallel threads.\n");
    printf("=====================================================\n");

    return 0;
}
