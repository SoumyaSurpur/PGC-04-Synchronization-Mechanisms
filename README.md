# PGC-04: Synchronization Mechanisms in Parallel Programs

> **Course:** Parallel and GPU Computing (PGC)  
> **Author:** Soumya Surpur  
> **USN:** `01FE24BCI121`  
> **Institution:** KLE Technological University  
> **Repository:** `PGC-04-Synchronization-Mechanisms`  

---

## Table of Contents
- [1. Aim and Objectives](#1-aim-and-objectives)
- [2. Understanding Synchronization in Simple Terms](#2-understanding-synchronization-in-simple-terms)
  - [2.1 What is a Race Condition?](#21-what-is-a-race-condition)
  - [2.2 Why Does counter++ Fail with Multiple Threads?](#22-why-does-counter-fail-with-multiple-threads)
  - [2.3 Synchronization Methods Used](#23-synchronization-methods-used)
- [3. Setup and Environment](#3-setup-and-environment)
- [4. Programs, How to Run, and Outputs](#4-programs-how-to-run-and-outputs)
  - [Program 1: race.c (No Synchronization)](#program-1-racec-no-synchronization)
  - [Program 2: critical.c (Critical Section)](#program-2-criticalc-critical-section)
  - [Program 3: atomic.c (Atomic Operation)](#program-3-atomicc-atomic-operation)
  - [Program 4: barrier.c (Barrier Coordination)](#program-4-barrierc-barrier-coordination)
  - [Program 5: semaphore.c (POSIX Semaphore)](#program-5-semaphorec-posix-semaphore)
  - [Program 6: compare_sync.c (Full Benchmark)](#program-6-compare_syncc-full-benchmark)
- [5. Creative Additions](#5-creative-additions)
  - [Creative 1: Lock-Free Reduction (reduction.c)](#creative-1-lock-free-reduction-reductionc)
  - [Creative 2: Explicit Locks & Non-Blocking Polling (omp_lock.c)](#creative-2-explicit-locks--non-blocking-polling-omp_lockc)
  - [Creative 3: Automated Visualizer (benchmark_visualizer.py)](#creative-3-automated-visualizer-benchmark_visualizerpy)
- [6. Performance Graphs](#6-performance-graphs)
- [7. Comparison Summary Table](#7-comparison-summary-table)
- [8. How to Compile and Run](#8-how-to-compile-and-run)
- [9. Conclusion](#9-conclusion)

---

## 1. Aim and Objectives

The goal of this experiment is to study how multiple threads work with shared data in memory:
1. See what happens when multiple threads update the same variable at the same time without any control (Race Condition).
2. Learn how to fix race conditions using different synchronization techniques:
   - **Critical Section** (`#pragma omp critical`)
   - **Atomic Operation** (`#pragma omp atomic`)
   - **Barrier** (`#pragma omp barrier`)
   - **Semaphore** (`sem_wait`, `sem_post` with Pthreads)
3. Compare the speed and accuracy of each technique across 1, 2, 4, and 8 threads for 10,000,000 increments.

---

## 2. Understanding Synchronization in Simple Terms

### 2.1 What is a Race Condition?
When two or more threads try to read and modify the same variable at the same time, the result depends on which thread runs first. If their operations overlap, updates get lost and the final answer becomes incorrect.

```text
Thread 1: Reads count = 100 ───────► Adds 1 ───────► Writes back 101
              ▲ (Overlap Window - Thread 2 reads before Thread 1 writes)
Thread 2: Reads count = 100 ───────► Adds 1 ───────► Writes back 101

Result: Two threads added 1, but count is only 101 instead of 102!
```

---

### 2.2 Why Does counter++ Fail with Multiple Threads?
Even though `counter++` looks like one simple line in C, the computer breaks it down into three separate steps:

```text
Step 1: Read 'counter' from RAM/Cache into CPU register  (MOV)
Step 2: Add 1 to the register value                     (ADD)
Step 3: Write the updated value back to RAM/Cache       (MOV)
```

If Thread A reads `counter` while Thread B is between Step 1 and Step 3, both threads read the same old number. When both write back their answers, one update completely overwrites the other.

---

### 2.3 Synchronization Methods Used

1. **No Synchronization (`race.c`):** Threads run freely with no locks. It is the fastest, but gives completely wrong results.
2. **Critical Section (`critical.c`):** Only one thread is allowed to enter the block at a time. All other threads wait in queue. It is 100% correct, but slow because threads spend time waiting.
3. **Atomic Operation (`atomic.c`):** Uses special CPU hardware instructions to update the variable in a single indivisible step. It is 100% correct and much faster than critical sections.
4. **Barrier (`barrier.c`):** A meeting point. All threads must reach the barrier before any thread can move to the next phase.
5. **POSIX Semaphore (`semaphore.c`):** An operating system counter. `sem_wait()` locks or decrements, and `sem_post()` unlocks or increments.

---

## 3. Setup and Environment

- **OS:** Ubuntu 22.04 LTS (running inside VMware Workstation)
- **Compiler:** GCC 15.2.0 with OpenMP support (`-fopenmp`) and POSIX threads (`-pthread`)

### Step 1: Install Build Tools
```bash
sudo apt update
sudo apt install build-essential -y
```
![System Setup](outputs/output_01_dependencies_install.jpeg)

### Step 2: Verify GCC and OpenMP
```bash
gcc --version
echo '#include <omp.h>' | gcc -E -fopenmp -xc - > /dev/null && echo OpenMP OK
```
![GCC and OpenMP Verification](outputs/output_02_gcc_openmp_verify.jpeg)

---

## 4. Programs, How to Run, and Outputs

### Program 1: race.c (No Synchronization)
- **File:** `src/race.c`
- **Workload:** 10,000,000 increments shared across threads without locks.

```c
#include <stdio.h>
#include <omp.h>
int main(){
    long counter = 0;
    #pragma omp parallel for
    for(long i = 0; i < 10000000; i++) counter++;
    printf("Counter=%ld\n", counter);
}
```

**Run Command:**
```bash
gcc src/race.c -fopenmp -o race
./race
```

**Output from Lab:**
```text
Counter=3630590
```
> **Observation:** Expected value was `10000000`, but we only got `3630590`. More than **63% of updates were lost** because threads overwrote each other's work!

#### Testing with Different Thread Counts:
```bash
OMP_NUM_THREADS=1 ./race   # Counter = 10000000 (100% correct, only 1 thread)
OMP_NUM_THREADS=2 ./race   # Counter = 5787832  (42% lost)
OMP_NUM_THREADS=4 ./race   # Counter = 4985599  (50% lost)
OMP_NUM_THREADS=8 ./race   # Counter = 4478782  (55% lost)
```
![Race Condition Scaling](outputs/output_06_race_degradation.jpeg)

---

### Program 2: critical.c (Critical Section)
- **File:** `src/critical.c`
- **Concept:** Uses `#pragma omp critical` to let only one thread increment the counter at any time.

```c
#include <stdio.h>
#include <omp.h>
int main(){
    long counter = 0;
    #pragma omp parallel for
    for(long i = 0; i < 10000000; i++){
        #pragma omp critical
        { counter++; }
    }
    printf("Counter=%ld\n", counter);
}
```

**Run Command:**
```bash
gcc src/critical.c -fopenmp -o critical
./critical
```

**Output from Lab:**
```text
Counter=10000000
```
> **Observation:** Result is 100% correct! However, as thread count increases from 1 to 8, execution time increases from `0.0904 s` to `0.6665 s` (~7x slower) because threads are stuck waiting in line.

---

### Program 3: atomic.c (Atomic Operation)
- **File:** `src/atomic.c`
- **Concept:** Uses `#pragma omp atomic` for direct hardware-level atomic updates.

```c
#include <stdio.h>
#include <omp.h>
int main(){
    long counter = 0;
    #pragma omp parallel for
    for(long i = 0; i < 10000000; i++){
        #pragma omp atomic
        counter++;
    }
    printf("Counter=%ld\n", counter);
}
```

**Run Command:**
```bash
gcc src/atomic.c -fopenmp -o atomic
./atomic
```

**Output from Lab:**
```text
Counter=10000000
```
> **Observation:** Result is 100% correct. At 8 threads, atomic finishes in `0.1296 s`, which is **5.1x faster than critical section (`0.6665 s`)** because it uses fast CPU instructions instead of heavy software locks.

---

### Program 4: barrier.c (Barrier Coordination)
- **File:** `src/barrier.c`
- **Concept:** Uses `#pragma omp barrier` to ensure all threads finish Phase 1 before any thread starts Phase 2.

```c
#include <stdio.h>
#include <omp.h>
int main(){
    #pragma omp parallel
    {
        printf("Thread %d before barrier\n", omp_get_thread_num());
        #pragma omp barrier
        printf("Thread %d after barrier\n", omp_get_thread_num());
    }
}
```

**Run Command:**
```bash
gcc src/barrier.c -fopenmp -o barrier
./barrier
```

**Output from Lab:**
```text
Thread 0 before barrier
Thread 3 before barrier
Thread 2 before barrier
Thread 1 before barrier
Thread 3 after barrier
Thread 0 after barrier
Thread 1 after barrier
Thread 2 after barrier
```
> **Observation:** Notice all threads print "before barrier" first. Only after all threads reach the barrier does any thread print "after barrier".

---

### Program 5: semaphore.c (POSIX Semaphore)
- **File:** `src/semaphore.c`
- **Concept:** 2 threads each increment a shared counter 100,000 times (total expected: 200,000) using a binary semaphore (`sem_wait` and `sem_post`).

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
sem_t semaphore;
int counter = 0;
void* work(void* a){
    for(int i = 0; i < 100000; i++){
        sem_wait(&semaphore);
        counter++;
        sem_post(&semaphore);
    }
    return NULL;
}
int main(){
    pthread_t t1, t2;
    sem_init(&semaphore, 0, 1);
    pthread_create(&t1, NULL, work, NULL);
    pthread_create(&t2, NULL, work, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Counter=%d\n", counter);
    sem_destroy(&semaphore);
}
```

**Run Command:**
```bash
gcc src/semaphore.c -pthread -o semaphore
./semaphore
```

**Output from Lab:**
```text
Counter=200000
```
> **Observation:** Result is 100% correct.

#### Terminal Run Screenshot (All Programs 1 to 5):
![Basic Programs Output](outputs/output_03_programs_execution.jpeg)

---

### Program 6: compare_sync.c (Full Benchmark)
- **File:** `src/compare_sync.c`
- **Concept:** Runs None, Critical, Atomic, and Lock together across 1, 2, 4, and 8 threads for 10,000,000 increments.

**Run Command:**
```bash
gcc src/compare_sync.c -fopenmp -o compare_sync
./compare_sync
```

**Benchmark Results Table (Recorded in Lab):**

| Mechanism | Threads | Final Count | Status | Time (seconds) |
| :--- | :---: | :---: | :---: | :---: |
| **None** | 1 | 10000000 | Correct | 0.0036 s |
| **Critical** | 1 | 10000000 | Correct | 0.0904 s |
| **Atomic** | 1 | 10000000 | Correct | 0.0536 s |
| **Lock** | 1 | 10000000 | Correct | 0.0895 s |
| **None** | 2 | 6357011 | **INCORRECT** | 0.0063 s |
| **Critical** | 2 | 10000000 | Correct | 0.3072 s |
| **Atomic** | 2 | 10000000 | Correct | 0.1273 s |
| **Lock** | 2 | 10000000 | Correct | 0.3490 s |
| **None** | 4 | 3209966 | **INCORRECT** | 0.0079 s |
| **Critical** | 4 | 10000000 | Correct | 0.6614 s |
| **Atomic** | 4 | 10000000 | Correct | 0.1271 s |
| **Lock** | 4 | 10000000 | Correct | 0.6085 s |
| **None** | 8 | 1968484 | **INCORRECT** | 0.0084 s |
| **Critical** | 8 | 10000000 | Correct | 0.6665 s |
| **Atomic** | 8 | 10000000 | Correct | **0.1296 s** |
| **Lock** | 8 | 10000000 | Correct | 0.6236 s |

![Benchmark Output](outputs/output_04_benchmark_matrix.jpeg)

---

## 5. Creative Additions

### Creative 1: Lock-Free Reduction (`reduction.c`)
- **File:** `src/reduction.c`
- **Idea:** What is the best way to sum numbers in parallel? Neither `atomic` nor `critical`! OpenMP provides `reduction(+:counter)`.
- **How it works:** Each thread gets its own private counter in CPU registers. There are no locks and zero waiting. At the end, OpenMP combines the private counts.
- **Result:** Finishes 10,000,000 increments in just **`0.0030 s`** at 8 threads—which is **222x faster than critical sections**!

### Creative 2: Explicit Locks & Non-Blocking Polling (`omp_lock.c`)
- **File:** `src/omp_lock.c`
- **Idea:** Demonstrates `omp_lock_t` and `omp_test_lock()`.
- If a lock is busy, instead of making the thread sit idle waiting, `omp_test_lock()` returns `0`, allowing the thread to do secondary useful work!

### Creative 3: Automated Visualizer (`benchmark_visualizer.py`)
- **File:** `scripts/benchmark_visualizer.py`
- Reads the recorded lab data and automatically generates clean graphs and CSV files in `results/`.

---

## 6. Performance Graphs

### Execution Time Comparison
Comparison of all 4 synchronization strategies across 1, 2, 4, and 8 threads:
![Execution Times](results/benchmark_execution_times.png)

### Race Condition Data Loss vs Threads
Shows how data loss increases as more threads are added:
![Data Loss Rate](results/race_condition_loss_rate.png)

### Lock Contention Overhead
Shows how waiting in line slows down Critical Section and Lock as thread count grows:
![Contention Slowdown](results/speedup_scaling_comparison.png)

---

## 7. Comparison Summary Table

| Mechanism | Correct? | Speed | Scalability | When to Use |
| :--- | :---: | :---: | :---: | :--- |
| **None (Race)** | No | Fastest | Fails | Never for shared mutable data |
| **Critical Section** | Yes | Slow | Poor | Complex multi-line blocks of code |
| **Atomic Operation** | Yes | Fast | Great | Simple variable updates like `x++` or `sum += val` |
| **OpenMP Lock** | Yes | Slow | Poor | Custom locking logic |
| **Barrier** | Yes | Depends | Good | Coordinating phases across threads |
| **POSIX Semaphore** | Yes | Moderate | Moderate | Controlling access to a fixed number of resources |
| **OpenMP Reduction** | Yes | **Fastest** | **Best** | **Parallel summations and counts** |

---

## 8. How to Compile and Run

To compile and run any program, use GCC in your Linux terminal:

```bash
# 1. Race condition
gcc src/race.c -fopenmp -o race
./race

# 2. Critical section
gcc src/critical.c -fopenmp -o critical
./critical

# 3. Atomic operation
gcc src/atomic.c -fopenmp -o atomic
./atomic

# 4. Barrier
gcc src/barrier.c -fopenmp -o barrier
./barrier

# 5. Semaphore
gcc src/semaphore.c -pthread -o semaphore
./semaphore

# 6. Benchmark comparison
gcc src/compare_sync.c -fopenmp -o compare_sync
./compare_sync

# 7. Creative Reduction
gcc src/reduction.c -fopenmp -o reduction
./reduction

# 8. Creative Explicit Lock
gcc src/omp_lock.c -fopenmp -o omp_lock
./omp_lock

# 9. Generate Python Charts
python3 scripts/benchmark_visualizer.py
```

Or run all programs automatically:
```bash
chmod +x scripts/run_all.sh
./scripts/run_all.sh
```

---

## 9. Conclusion

1. **Unsynchronized access (`race.c`)** is fast but unreliable: over 55% of data was lost when running on 8 threads.
2. **Critical sections (`critical.c`)** guarantee correctness, but cause heavy thread queuing, running 7.3x slower at 8 threads.
3. **Atomic operations (`atomic.c`)** use CPU hardware instructions, making them **5.1x faster than critical sections** at 8 threads.
4. **Barriers (`barrier.c`)** coordinate execution stages so that all threads finish Phase 1 before Phase 2 begins.
5. **OpenMP Reduction (`reduction.c`)** is the fastest solution for summations, providing **222x faster execution than critical sections** through private per-thread accumulators.
