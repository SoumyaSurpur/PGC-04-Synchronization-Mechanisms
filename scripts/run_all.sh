#!/usr/bin/env bash
# ==============================================================================
# run_all.sh - Automated Compilation, Execution & Verification Script
# Course: Parallel and GPU Computing (PGC)
# Author: Soumya Surpur (USN: 01FE24BCI121)
# ==============================================================================

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}================================================================${NC}"
echo -e "${GREEN}  PGC-04: Synchronization Mechanisms in Parallel Programs        ${NC}"
echo -e "${YELLOW}  Author: Soumya Surpur | USN: 01FE24BCI121 | KLE Tech           ${NC}"
echo -e "${BLUE}================================================================${NC}"

# Check GCC
if ! command -v gcc &> /dev/null; then
    echo -e "${RED}[!] Error: GCC is not installed. Run: sudo apt install build-essential -y${NC}"
    exit 1
fi

echo -e "\n${YELLOW}[*] Compiling all C synchronization source files...${NC}"
mkdir -p bin
gcc -Wall -O2 -fopenmp src/race.c -o bin/race
gcc -Wall -O2 -fopenmp src/critical.c -o bin/critical
gcc -Wall -O2 -fopenmp src/atomic.c -o bin/atomic
gcc -Wall -O2 -fopenmp src/barrier.c -o bin/barrier
gcc -Wall -O2 -pthread src/semaphore.c -o bin/semaphore
gcc -Wall -O2 -fopenmp src/compare_sync.c -o bin/compare_sync
gcc -Wall -O2 -fopenmp src/omp_lock.c -o bin/omp_lock
gcc -Wall -O2 -fopenmp src/reduction.c -o bin/reduction
echo -e "${GREEN}[+] All programs compiled successfully into bin/${NC}"

echo -e "\n${BLUE}--- [1/6] Running race.c (Unsynchronized Access) ---${NC}"
./bin/race

echo -e "\n${BLUE}--- [2/6] Running critical.c (OpenMP Critical Section) ---${NC}"
./bin/critical

echo -e "\n${BLUE}--- [3/6] Running atomic.c (OpenMP Hardware Atomic) ---${NC}"
./bin/atomic

echo -e "\n${BLUE}--- [4/6] Running barrier.c (OpenMP Phased Barrier) ---${NC}"
./bin/barrier

echo -e "\n${BLUE}--- [5/6] Running semaphore.c (POSIX Binary Semaphore) ---${NC}"
./bin/semaphore

echo -e "\n${BLUE}--- [6/6] Running compare_sync.c (Full Scalability Benchmark) ---${NC}"
./bin/compare_sync

echo -e "\n${BLUE}--- [Creative Extension] Running reduction.c (Lock-Free Reduction) ---${NC}"
./bin/reduction

echo -e "\n${BLUE}--- [Creative Extension] Running omp_lock.c (Explicit Locks) ---${NC}"
./bin/omp_lock

echo -e "\n${GREEN}================================================================${NC}"
echo -e "${GREEN}  All synchronization benchmarks completed successfully!       ${NC}"
echo -e "${GREEN}================================================================${NC}"
