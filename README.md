# fork-or-starve
### Dining Philosophers Concurrency Simulator

---

## Overview

A multithreaded simulation of the classic Dining Philosophers problem implemented in C using POSIX threads, mutexes, and semaphores. Five philosopher threads are created with `pthread_create()`, competing for five shared fork resources modeled as `pthread_mutex_t` mutexes. The waiter solution uses a `sem_t` counting semaphore to limit concurrent resource access. The simulator demonstrates deadlock formation and compares two prevention strategies side by side with a live terminal visualization updating every 0.5 seconds.

---

## Requirements

- Linux (course VM)
- GCC compiler
- POSIX threads library (pthread)
- POSIX semaphore library (sem)

---

## Build Instructions

```bash
# Clone the repo
git clone https://github.com/[username]/fork-or-starve.git
cd fork-or-starve

# Build the project
make

# Clean build files
make clean
```

---

## Usage

```bash
# Run naive mode — will deadlock (mode=0)
./dining_philosophers --mode=0

# Run asymmetric solution — no deadlock (mode=1)
./dining_philosophers --mode=1

# Run waiter semaphore solution — no deadlock (mode=2)
./dining_philosophers --mode=2

# Run for a specific number of steps
./dining_philosophers --mode=2 --steps=200

# Show help
./dining_philosophers --help
```

---

## Modes

### mode=0 — Naive (No Solution)
All philosophers acquire their left `pthread_mutex_t` fork first, then their right. Philosopher 1 acquires its left fork mutex, then attempts its right fork mutex — deadlock occurs when all five do the same simultaneously. This demonstrates all four necessary conditions for deadlock.

| Deadlock Condition | How It Appears |
|---|---|
| Mutual Exclusion | Only one philosopher can hold a fork at a time |
| Hold and Wait | Philosopher holds left fork while waiting for right |
| No Preemption | A fork cannot be forcibly taken from a neighbor |
| Circular Wait | P0 waits on P1 → P1 on P2 → P2 on P3 → P3 on P4 → P4 on P0 |

Expected output: Deadlock detected within ~10 cycles. All philosophers stuck in HUNGRY state.

---

### mode=1 — Asymmetric Solution
Philosopher 5 (P4) picks up the right fork first instead of the left. This breaks the circular wait condition — P3 and P4 now compete for the same fork, so one of them will always succeed and make forward progress.

Expected output: All philosophers eat. No deadlock detected.

---

### mode=2 — Waiter (Semaphore Solution)
A `sem_t` counting semaphore initialized to 4 limits how many philosophers can attempt to acquire forks simultaneously. Since at most four of five philosophers compete at once, at least one philosopher will always successfully acquire both forks, mathematically guaranteeing no deadlock.

Expected output: All philosophers eat. Semaphore never exceeds 4 concurrent. No deadlock detected.

---

## OS Primitives

| Primitive | Purpose |
|---|---|
| `pthread_create()` | Creates one philosopher thread per philosopher (5 total) |
| `pthread_join()` | Main thread waits for all philosopher threads to finish |
| `pthread_mutex_t` | One mutex per fork — enforces mutual exclusion |
| `pthread_mutex_lock()` | Philosopher claims a fork (blocks if already held) |
| `pthread_mutex_unlock()` | Philosopher releases a fork when done eating |
| `sem_t` | Counting semaphore initialized to 4 for waiter solution |
| `sem_wait()` | Decrements semaphore — blocks if 4 philosophers already competing |
| `sem_post()` | Increments semaphore — signals next philosopher can attempt forks |

---

## Thread State Machine

Each philosopher thread continuously cycles through three states. Every state change updates the shared `state[i]` variable under mutex protection before changing fork ownership, preventing race conditions.

```
THINKING → HUNGRY → (wait for forks) → EATING → THINKING → ...
```

```c
pthread_mutex_lock(&state_mutex);
state[id] = HUNGRY;             /* update state before fork attempt */
pthread_mutex_unlock(&state_mutex);

pick_up_forks(id);              /* acquire forks */

pthread_mutex_lock(&state_mutex);
state[id] = EATING;             /* confirm eating after forks acquired */
pthread_mutex_unlock(&state_mutex);
```

---

## Algorithms

### mode=0 — Naive Algorithm
```c
pthread_mutex_lock(&forks[left(i)]);
pthread_mutex_lock(&forks[right(i)]);  /* all 5 stuck here = DEADLOCK */
/* eat */
pthread_mutex_unlock(&forks[right(i)]);
pthread_mutex_unlock(&forks[left(i)]);
```

### mode=1 — Asymmetric Algorithm
```c
if (id == NUM_PHILOSOPHERS - 1) {          /* Philosopher 5 (P4) only */
    pthread_mutex_lock(&forks[right(id)]); /* right first */
    pthread_mutex_lock(&forks[left(id)]);
} else {
    pthread_mutex_lock(&forks[left(id)]);  /* all others: left first */
    pthread_mutex_lock(&forks[right(id)]);
}
```

### mode=2 — Waiter Algorithm
```c
sem_wait(&waiter);                     /* block if 4 already competing */
pthread_mutex_lock(&forks[left(id)]);
pthread_mutex_lock(&forks[right(id)]);
/* eat */
pthread_mutex_unlock(&forks[right(id)]);
pthread_mutex_unlock(&forks[left(id)]);
sem_post(&waiter);                     /* release seat at table */
```

### Deadlock Detection
A monitor checks `state[i]` for all philosophers on every cycle. If all five are simultaneously HUNGRY and none are EATING, a deadlock is confirmed and reported.

---

## Input

The simulator takes no interactive input during runtime. All configuration is passed through command line arguments when launching the program.

| Argument | Values | Description |
|---|---|---|
| `--mode` | `0`, `1`, `2` | Sets the synchronization mode (0=naive, 1=asymmetric, 2=waiter) |
| `--steps` | Any positive integer | How many simulation cycles to run (default: 50) |
| `--help` | — | Prints usage instructions and exits |

If no arguments are provided the simulator defaults to `--mode=0 --steps=50`. If an unrecognized argument is passed, the program prints a friendly error message and exits cleanly.

---

## Output

The simulator produces a live state table updating every 0.5 seconds, and a final statistics summary when the simulation ends.

### Live State Table

```
──────────────────────────────────────────────────────────
 PHILOSOPHER    STATE        LEFT FORK   RIGHT FORK   MEALS
──────────────────────────────────────────────────────────
 P0 Aristotle   EATING       F0:HELD     F1:HELD      3
 P1 Plato       THINKING     F1:free     F2:free      2
 P2 Socrates    HUNGRY       F2:free     F3:free      2
 P3 Descartes   EATING       F3:HELD     F4:HELD      3
 P4 Kant        THINKING     F4:free     F0:free      2
──────────────────────────────────────────────────────────
```

| Color | State |
|---|---|
| Green | EATING |
| Red | HUNGRY |
| Blue | THINKING |

### Deadlock Alert
```
*** DEADLOCK DETECTED — all philosophers waiting, none eating ***
```

### Starvation Warning
```
*** STARVATION WARNING — P2 Socrates meal count lagging significantly ***
```

### Final Statistics Summary
```
── STATISTICS ────────────────────────────────────────────
 Total meals eaten  : 12
 Deadlocks detected : 0
 Simulation steps   : 50
 Meals per philosopher:
   P0 Aristotle  : 3
   P1 Plato      : 2
   P2 Socrates   : 2
   P3 Descartes  : 3
   P4 Kant       : 2
──────────────────────────────────────────────────────────
```

In mode=0 all meal counts will be 0 since no philosopher eats before deadlock. In mode=1 and mode=2 counts are distributed across all philosophers.

---

## Fairness and Starvation

Fairness is measured by tracking meal counts per philosopher throughout the simulation. After every cycle the stats module compares each philosopher's meal count against the group average. If any philosopher's count lags significantly behind neighbors a starvation warning is printed to the terminal. Starvation is most likely in mode=1 under heavy load — watch P3 and P4 meal counts closely.

---

## File Structure

```
fork-or-starve/
│
├── Makefile
├── README.txt
│
├── src/
│   ├── main.c          — entry point, CLI argument parsing
│   ├── philosopher.c   — thread logic, state machine, state[i] tracking
│   ├── philosopher.h
│   ├── forks.c         — mutex init, mode=0 naive, mode=1 asymmetric
│   ├── forks.h
│   ├── semaphore.c     — mode=2 waiter semaphore solution
│   ├── semaphore.h
│   ├── display.c       — terminal output, ANSI colors, 0.5s refresh
│   ├── display.h
│   ├── stats.c         — meal counts, starvation detection, deadlock log
│   └── stats.h
│
└── docs/
    ├── test_plan.txt
   
```

---

## OS Concepts Demonstrated

| Concept | Where |
|---------|-------|
| POSIX Threads (`pthread_create()`) | One thread per philosopher |
| Mutex Locks (`pthread_mutex_t`) | One mutex per fork |
| Counting Semaphore (`sem_t`) | mode=2 waiter — limits concurrent access to 4 |
| Deadlock | All 4 conditions shown in mode=0 |
| Deadlock Prevention | mode=1 asymmetric and mode=2 waiter solutions |
| Starvation Detection | Meal count comparison in statistics output |
| Race Conditions | Prevented via state[i] mutex protection |

---

## Known Limitations

- Starvation is possible in mode=1 under heavy load
- Simulation uses sleep-based timing rather than real concurrency scheduling
- Number of philosophers is fixed at 5 (configurable via #define)

---
