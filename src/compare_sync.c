/*
 * =====================================================================================
 * Program 06: compare_sync.c
 * Description: Empirical benchmarking and scalability comparison of four shared
 *              counter update strategies:
 *                1. None (Unsynchronized / Race Condition)
 *                2. Critical Section (#pragma omp critical)
 *                3. Hardware Atomic Operation (#pragma omp atomic)
 *                4. OpenMP Mutex Lock (omp_lock_t)
 *              across 1, 2, 4, and 8 threads for 10,000,000 shared increments.
 * Course:      Parallel and GPU Computing (PGC)
 * Author:      Soumya Surpur (USN: 01FE24BCI121)
 * Institution: KLE Technological University
 * =====================================================================================
 * Compilation: gcc compare_sync.c -fopenmp -o compare_sync
 * Execution:   ./compare_sync
 * =====================================================================================
 * Empirical Lab Results (Captured in output_04_benchmark_matrix.jpeg):
 * -------------------------------------------------------------------------------------
 * Mechanism  | Threads | Final Count  | Correctness |   Time (s)
 * -------------------------------------------------------------------------------------
 * None       |       1 |     10000000 |     Correct |     0.0036
 * Critical   |       1 |     10000000 |     Correct |     0.0904
 * Atomic     |       1 |     10000000 |     Correct |     0.0536
 * Lock       |       1 |     10000000 |     Correct |     0.0895
 * -------------------------------------------------------------------------------------
 * None       |       2 |      6357011 |   INCORRECT |     0.0063
 * Critical   |       2 |     10000000 |     Correct |     0.3072
 * Atomic     |       2 |     10000000 |     Correct |     0.1273
 * Lock       |       2 |     10000000 |     Correct |     0.3490
 * -------------------------------------------------------------------------------------
 * None       |       4 |      3209966 |   INCORRECT |     0.0079
 * Critical   |       4 |     10000000 |     Correct |     0.6614
 * Atomic     |       4 |     10000000 |     Correct |     0.1271
 * Lock       |       4 |     10000000 |     Correct |     0.6085
 * -------------------------------------------------------------------------------------
 * None       |       8 |      1968484 |   INCORRECT |     0.0084
 * Critical   |       8 |     10000000 |     Correct |     0.6665
 * Atomic     |       8 |     10000000 |     Correct |     0.1296
 * Lock       |       8 |     10000000 |     Correct |     0.6236
 * -------------------------------------------------------------------------------------
 * =====================================================================================
 * Scalability & Performance Analysis:
 * 1. None: Blazing fast (0.0036s - 0.0084s), but corrupted. As thread count increases,
 *    accuracy drops drastically from 100% (1T) -> 63.5% (2T) -> 32.1% (4T) -> 19.7% (8T).
 * 2. Critical Section: 100% correct, but suffers severe lock contention. At 8 threads,
 *    it takes 0.6665s (~7.4x slower than 1 thread). Contention completely destroys scalability.
 * 3. Lock (omp_lock_t): Similar runtime to critical section (0.6236s at 8 threads) because
 *    it also serializes memory access using software lock routines.
 * 4. Atomic: The clear winner for scalar updates! Takes only 0.1296s at 8 threads,
 *    outperforming Critical by 5.14x and Lock by 4.81x due to hardware-level CPU bus/cache locking.
 * =====================================================================================
 */

#include <stdio.h>
#include <omp.h>

#define N 10000000L

/* Strategy 1: Unsynchronized access */
long run_none(int nthreads) {
    long counter = 0;
    omp_set_num_threads(nthreads);
    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        counter++;
    }
    return counter;
}

/* Strategy 2: OpenMP Critical Section */
long run_critical(int nthreads) {
    long counter = 0;
    omp_set_num_threads(nthreads);
    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        #pragma omp critical
        {
            counter++;
        }
    }
    return counter;
}

/* Strategy 3: Hardware Atomic Operation */
long run_atomic(int nthreads) {
    long counter = 0;
    omp_set_num_threads(nthreads);
    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        #pragma omp atomic
        counter++;
    }
    return counter;
}

/* Strategy 4: OpenMP Coarse-Grained Lock */
long run_lock(int nthreads) {
    long counter = 0;
    omp_lock_t lock;
    omp_init_lock(&lock);
    omp_set_num_threads(nthreads);
    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        omp_set_lock(&lock);
        counter++;
        omp_unset_lock(&lock);
    }
    omp_destroy_lock(&lock);
    return counter;
}

/* Helper to execute and print benchmark results */
void run_and_report(const char *name, long (*fn)(int), int nthreads) {
    double start = omp_get_wtime();
    long result = fn(nthreads);
    double elapsed = omp_get_wtime() - start;
    const char *correctness = (result == N) ? "Correct" : "INCORRECT";
    printf("%-10s | %7d | %12ld | %11s | %10.4f\n",
           name, nthreads, result, correctness, elapsed);
}

int main(void) {
    int thread_counts[] = {1, 2, 4, 8};
    int num_configs = sizeof(thread_counts) / sizeof(thread_counts[0]);

    printf("===================================================================\n");
    printf(" Comparing synchronization mechanisms for %ld shared increments\n", N);
    printf("===================================================================\n\n");
    printf("Mechanism  | Threads | Final Count  | Correctness |   Time (s)\n");
    printf("-------------------------------------------------------------------\n");

    for (int i = 0; i < num_configs; i++) {
        int t = thread_counts[i];
        run_and_report("None",     run_none,     t);
        run_and_report("Critical", run_critical, t);
        run_and_report("Atomic",   run_atomic,   t);
        run_and_report("Lock",     run_lock,     t);
        printf("-------------------------------------------------------------------\n");
    }

    printf("\nExpected correct final count = %ld\n", N);
    printf("Observations:\n");
    printf(" 1. 'None' is fastest but produces catastrophic race conditions.\n");
    printf(" 2. 'Critical' and 'Lock' serialize execution, causing heavy contention.\n");
    printf(" 3. 'Atomic' provides high throughput via hardware instructions (5.1x faster at 8T).\n");
    printf("===================================================================\n");

    return 0;
}
