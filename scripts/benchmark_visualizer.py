#!/usr/bin/env python3
"""
benchmark_visualizer.py
Visualizes empirical performance, scalability, and data loss rates across
different shared-memory synchronization mechanisms in OpenMP and Pthreads.

Author: Soumya Surpur (USN: 01FE24BCI121)
Institution: KLE Technological University
Course: Parallel and GPU Computing (PGC)
"""

import os
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

def generate_visualizations():
    os.makedirs("results", exist_ok=True)
    
    # -------------------------------------------------------------------------
    # 1. Dataset: Empirical Lab Benchmark Data (Captured from Lab Outputs)
    # -------------------------------------------------------------------------
    threads = [1, 2, 4, 8]
    
    data = {
        "Threads": [1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 8, 8, 8, 8],
        "Mechanism": [
            "None", "Critical", "Atomic", "Lock",
            "None", "Critical", "Atomic", "Lock",
            "None", "Critical", "Atomic", "Lock",
            "None", "Critical", "Atomic", "Lock"
        ],
        "Final_Count": [
            10000000, 10000000, 10000000, 10000000,
            6357011,  10000000, 10000000, 10000000,
            3209966,  10000000, 10000000, 10000000,
            1968484,  10000000, 10000000, 10000000
        ],
        "Correctness": [
            "Correct", "Correct", "Correct", "Correct",
            "INCORRECT", "Correct", "Correct", "Correct",
            "INCORRECT", "Correct", "Correct", "Correct",
            "INCORRECT", "Correct", "Correct", "Correct"
        ],
        "Time_s": [
            0.0036, 0.0904, 0.0536, 0.0895,
            0.0063, 0.3072, 0.1273, 0.3490,
            0.0079, 0.6614, 0.1271, 0.6085,
            0.0084, 0.6665, 0.1296, 0.6236
        ]
    }
    
    df = pd.DataFrame(data)
    df.to_csv("results/sync_benchmarks.csv", index=False)
    print("[+] Saved benchmark dataset to results/sync_benchmarks.csv")

    # Race degradation dataset (From screenshot 06)
    race_data = {
        "Threads": [1, 2, 4, 8],
        "Final_Counter": [10000000, 5787832, 4985599, 4478782],
        "Expected": [10000000, 10000000, 10000000, 10000000],
        "Lost_Updates": [0, 4212168, 5014401, 5521218],
        "Loss_Percentage": [0.0, 42.12, 50.14, 55.21]
    }
    df_race = pd.DataFrame(race_data)
    df_race.to_csv("results/race_scaling.csv", index=False)
    print("[+] Saved race degradation dataset to results/race_scaling.csv")

    # -------------------------------------------------------------------------
    # Chart 1: Execution Time by Synchronization Mechanism across Thread Counts
    # -------------------------------------------------------------------------
    plt.figure(figsize=(10, 6), dpi=300)
    plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')

    bar_width = 0.18
    r1 = np.arange(len(threads))
    r2 = [x + bar_width for x in r1]
    r3 = [x + bar_width for x in r2]
    r4 = [x + bar_width for x in r3]

    t_none = df[df["Mechanism"] == "None"]["Time_s"].values
    t_critical = df[df["Mechanism"] == "Critical"]["Time_s"].values
    t_atomic = df[df["Mechanism"] == "Atomic"]["Time_s"].values
    t_lock = df[df["Mechanism"] == "Lock"]["Time_s"].values

    plt.bar(r1, t_none, width=bar_width, color="#ef4444", edgecolor="black", alpha=0.85, label="None (Race - Incorrect)")
    plt.bar(r2, t_critical, width=bar_width, color="#f97316", edgecolor="black", alpha=0.85, label="Critical (#pragma omp critical)")
    plt.bar(r3, t_atomic, width=bar_width, color="#10b981", edgecolor="black", alpha=0.85, label="Atomic (#pragma omp atomic)")
    plt.bar(r4, t_lock, width=bar_width, color="#3b82f6", edgecolor="black", alpha=0.85, label="Lock (omp_lock_t)")

    plt.xlabel("Thread Concurrency (Threads)", fontweight="bold", fontsize=12)
    plt.ylabel("Execution Time (Seconds) - Lower is Better", fontweight="bold", fontsize=12)
    plt.title("Execution Time Comparison Across Synchronization Strategies\n(10,000,000 Shared Increments)", fontsize=14, fontweight="bold", pad=15)
    plt.xticks([r + 1.5 * bar_width for r in range(len(threads))], ['1 Thread', '2 Threads', '4 Threads', '8 Threads'], fontsize=11)
    plt.legend(frameon=True, fontsize=10)
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    # Annotate Atomic vs Critical speedup at 8 threads
    diff_ratio = t_critical[-1] / t_atomic[-1]
    plt.annotate(f"Atomic is {diff_ratio:.2f}x Faster\nthan Critical (8 Threads)",
                 xy=(r3[-1], t_atomic[-1]), xytext=(r3[-1] - 0.2, 0.40),
                 arrowprops=dict(facecolor='#10b981', shrink=0.08, width=1.5, headwidth=6),
                 fontsize=10, fontweight='bold', color="#065f46",
                 bbox=dict(boxstyle="round,pad=0.4", fc="#d1fae5", ec="#10b981", lw=1))

    plt.tight_layout()
    plt.savefig("results/benchmark_execution_times.png")
    plt.close()
    print("[+] Generated results/benchmark_execution_times.png")

    # -------------------------------------------------------------------------
    # Chart 2: Race Condition Data Loss & Corruption vs Thread Scaling
    # -------------------------------------------------------------------------
    plt.figure(figsize=(9, 5.5), dpi=300)
    plt.plot(df_race["Threads"], df_race["Loss_Percentage"], marker='o', color='#dc2626', linewidth=2.5, markersize=8, label="Lost Updates (%)")
    
    for i, row in df_race.iterrows():
        plt.text(row["Threads"], row["Loss_Percentage"] + 1.8, 
                 f"{row['Loss_Percentage']:.1f}%\n({row['Final_Counter']:,})", 
                 ha='center', fontsize=9, fontweight='bold', color='#991b1b')

    plt.xlabel("Number of Concurrent Threads", fontweight="bold", fontsize=12)
    plt.ylabel("Data Loss Percentage (%)", fontweight="bold", fontsize=12)
    plt.title("Catastrophic Data Loss in Unsynchronized Multi-Threaded Loop\n(10M Target Increments - Screenshot 06 Empirical Data)", fontsize=13, fontweight="bold", pad=12)
    plt.ylim(-5, 70)
    plt.xticks(threads, ['1 Thread', '2 Threads', '4 Threads', '8 Threads'], fontsize=11)
    plt.axhline(0, color='gray', linestyle='--', alpha=0.5)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend(loc="upper left", fontsize=10)
    plt.tight_layout()
    plt.savefig("results/race_condition_loss_rate.png")
    plt.close()
    print("[+] Generated results/race_condition_loss_rate.png")

    # -------------------------------------------------------------------------
    # Chart 3: Contention Bottleneck - Execution Slowdown Factor vs 1 Thread
    # -------------------------------------------------------------------------
    plt.figure(figsize=(9, 5.5), dpi=300)
    
    crit_overhead = t_critical / t_critical[0]
    lock_overhead = t_lock / t_lock[0]
    atom_overhead = t_atomic / t_atomic[0]
    
    plt.plot(threads, crit_overhead, marker='s', linewidth=2.5, color='#f97316', label="Critical Overhead Factor")
    plt.plot(threads, lock_overhead, marker='^', linewidth=2.5, color='#3b82f6', label="Lock Overhead Factor")
    plt.plot(threads, atom_overhead, marker='o', linewidth=2.5, color='#10b981', label="Atomic Overhead Factor")

    for x, y in zip(threads, crit_overhead):
        plt.annotate(f"{y:.2f}x", (x, y), textcoords="offset points", xytext=(0, 8), ha='center', fontsize=9, fontweight='bold', color='#c2410c')

    plt.xlabel("Number of Concurrent Threads", fontweight="bold", fontsize=12)
    plt.ylabel("Slowdown Factor (Relative to 1 Thread)", fontweight="bold", fontsize=12)
    plt.title("Contention Bottleneck: Lock Queuing Overhead vs Thread Count\n(Higher factor indicates worse lock queuing)", fontsize=13, fontweight="bold", pad=12)
    plt.xticks(threads, ['1 Thread', '2 Threads', '4 Threads', '8 Threads'], fontsize=11)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend(fontsize=10)
    plt.tight_layout()
    plt.savefig("results/speedup_scaling_comparison.png")
    plt.close()
    print("[+] Generated results/speedup_scaling_comparison.png")

if __name__ == "__main__":
    generate_visualizations()
