/*
 * =====================================================================================
 * Program 01: race.c
 * Description: Demonstrates data hazards and race conditions in shared-memory
 *              multithreaded programming using OpenMP without synchronization.
 * Course:      Parallel and GPU Computing (PGC)
 * Author:      Soumya Surpur (USN: 01FE24BCI121)
 * Institution: KLE Technological University
 * =====================================================================================
 * Compilation: gcc race.c -fopenmp -o race
 * Execution:   ./race
 *              OMP_NUM_THREADS=4 ./race
 *              export OMP_NUM_THREADS=8; ./race
 * =====================================================================================
 * Observed Lab Outputs (from terminal outputs):
 *   - Default (Multi-threaded execution): Counter = 3630590 (Expected: 10000000)
 *   - OMP_NUM_THREADS=1:                  Counter = 10000000 (100% correct, no concurrency)
 *   - OMP_NUM_THREADS=2:                  Counter = 5787832  (Lost ~42.1% updates)
 *   - OMP_NUM_THREADS=4:                  Counter = 4985599 / 6652815 (~33% - 50% loss)
 *   - OMP_NUM_THREADS=8:                  Counter = 2252149 / 4478782 (~55% - 77.5% loss)
 * =====================================================================================
 * Key Theoretical Insights:
 * 1. Read-Modify-Write Hazard: 'counter++' is not an indivisible machine instruction.
 *    It translates into three distinct assembly instructions:
 *      (a) MOV [counter], %eax  (Read value into register)
 *      (b) ADD $1, %eax         (Increment register)
 *      (c) MOV %eax, [counter]  (Write value back to memory)
 * 2. Unsynchronized Interleaving: When two threads read the identical initial value
 *    before either writes back the result, one increment is silently overwritten.
 * 3. Scaling Inaccuracy: As the thread count increases, CPU core contention and
 *    temporal interleaving increase exponentially, resulting in severe data corruption.
 * =====================================================================================
 */

#include <stdio.h>
#include <omp.h>

#define N 10000000L

int main(void) {
    long counter = 0;

    printf("=====================================================\n");
    printf(" Experiment: Race Condition (No Synchronization)\n");
    printf(" Workload:   %ld Shared Increments\n", N);
    printf("=====================================================\n");

    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        counter++; // Critical section left unprotected: Race Condition!
    }

    double elapsed_time = omp_get_wtime() - start_time;

    printf("Final Counter Value : %ld\n", counter);
    printf("Expected Value      : %ld\n", N);
    printf("Lost Updates Count  : %ld (%.2f%% lost)\n", 
           N - counter, ((double)(N - counter) / N) * 100.0);
    printf("Execution Time      : %.6f seconds\n", elapsed_time);
    printf("Status              : %s\n", (counter == N) ? "CORRECT" : "INCORRECT (RACE DETECTED)");
    printf("=====================================================\n");

    return 0;
}
