/*
 * =====================================================================================
 * Program 03: atomic.c
 * Description: Implements lightweight hardware-supported atomic operations using
 *              '#pragma omp atomic' for concurrent shared-variable updates.
 * Course:      Parallel and GPU Computing (PGC)
 * Author:      Soumya Surpur (USN: 01FE24BCI121)
 * Institution: KLE Technological University
 * =====================================================================================
 * Compilation: gcc atomic.c -fopenmp -o atomic
 * Execution:   ./atomic
 *              OMP_NUM_THREADS=4 ./atomic
 * =====================================================================================
 * Observed Lab Outputs (from terminal outputs):
 *   - Default Run:         Counter = 10000000 (100% Correct)
 *   - OMP_NUM_THREADS=4:   Counter = 10000000 (100% Correct)
 *   - Measured Timings:    1 Thread: 0.0536s | 2 Threads: 0.1273s | 4 Threads: 0.1271s | 8 Threads: 0.1296s
 * =====================================================================================
 * Key Theoretical Insights:
 * 1. Hardware-Level Synchronization: Translates to direct CPU atomic memory primitives
 *    (such as 'LOCK XADD' or 'LOCK CMPXCHG' on x86-64 architecture).
 * 2. Minimal Overhead: Bypasses the heavy kernel-level mutex locking constructs and
 *    thread descheduling queues required by '#pragma omp critical'.
 * 3. Outstanding Scalability: Execution latency stays virtually constant across 2, 4,
 *    and 8 threads (~0.127s - 0.129s). At 8 threads, it is ~5.14x faster than critical
 *    sections (0.1296s vs 0.6665s).
 * 4. Limitation: Only applies to single scalar read-modify-write memory operations,
 *    not arbitrary structured code blocks.
 * =====================================================================================
 */

#include <stdio.h>
#include <omp.h>

#define N 10000000L

int main(void) {
    long counter = 0;

    printf("=====================================================\n");
    printf(" Experiment: Atomic Operation Synchronization\n");
    printf(" Directive:  #pragma omp atomic\n");
    printf(" Workload:   %ld Shared Increments\n", N);
    printf("=====================================================\n");

    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        #pragma omp atomic
        counter++; // Hardware-supported indivisible memory update
    }

    double elapsed_time = omp_get_wtime() - start_time;

    printf("Final Counter Value : %ld\n", counter);
    printf("Expected Value      : %ld\n", N);
    printf("Execution Time      : %.6f seconds\n", elapsed_time);
    printf("Status              : %s\n", (counter == N) ? "CORRECT" : "INCORRECT");
    printf("Observation         : Hardware atomic locking provides high throughput & low contention.\n");
    printf("=====================================================\n");

    return 0;
}
