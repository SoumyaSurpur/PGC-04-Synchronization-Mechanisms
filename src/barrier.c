/*
 * =====================================================================================
 * Program 04: barrier.c
 * Description: Demonstrates phased synchronization and thread coordination using
 *              the OpenMP '#pragma omp barrier' directive.
 * Course:      Parallel and GPU Computing (PGC)
 * Author:      Soumya Surpur (USN: 01FE24BCI121)
 * Institution: KLE Technological University
 * =====================================================================================
 * Compilation: gcc barrier.c -fopenmp -o barrier
 * Execution:   ./barrier
 *              OMP_NUM_THREADS=4 ./barrier
 * =====================================================================================
 * Observed Lab Outputs (from output_03_programs_execution.jpeg):
 *   Thread 0 before barrier
 *   Thread 3 before barrier
 *   Thread 2 before barrier
 *   Thread 1 before barrier
 *   Thread 3 after barrier
 *   Thread 0 after barrier
 *   Thread 1 after barrier
 *   Thread 2 after barrier
 * =====================================================================================
 * Key Theoretical Insights:
 * 1. Phased Execution Rendezvous: A barrier enforces an explicit checkpoint. Every thread
 *    in the team must reach the '#pragma omp barrier' directive before ANY thread is
 *    permitted to advance past it.
 * 2. Multi-Stage Computational Integrity: Essential in multi-phase algorithms (e.g.,
 *    timestepping simulations, mesh computations, matrix decompositions) where Phase 2
 *    requires the complete, verified output of Phase 1 from all parallel workers.
 * 3. Straggler Latency: If one thread runs slower than others (due to uneven workloads,
 *    cache misses, or OS scheduling preemption), all other threads must idle at the barrier.
 * 4. Distinct Role: Unlike critical/atomic/mutex primitives, barriers do not protect
 *    individual shared variables; they synchronize overall algorithm control flow.
 * =====================================================================================
 */

#include <stdio.h>
#include <omp.h>

int main(void) {
    printf("=====================================================\n");
    printf(" Experiment: Phased Barrier Synchronization\n");
    printf(" Directive:  #pragma omp barrier\n");
    printf("=====================================================\n\n");

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int num_threads = omp_get_num_threads();

        // Phase 1: Pre-barrier computational phase
        printf("Thread %d of %d: arrived BEFORE barrier (Phase 1)\n", tid, num_threads);

        // Explicit synchronization checkpoint
        #pragma omp barrier

        // Phase 2: Post-barrier computational phase (Guaranteed all Phase 1 tasks finished)
        printf("Thread %d of %d: released AFTER barrier (Phase 2)\n", tid, num_threads);
    }

    printf("\n=====================================================\n");
    printf("Verification: All threads completed Phase 1 before\n");
    printf("              any thread entered Phase 2.\n");
    printf("=====================================================\n");

    return 0;
}
