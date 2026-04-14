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

# Run asymmetric with odd philosophers picking right fork first (default)
./dining_philosophers --mode=1 --asy=odd

# Run asymmetric with even philosophers picking right fork first
./dining_philosophers --mode=1 --asy=even

# Run waiter semaphore solution — no deadlock (mode=2)
./dining_philosophers --mode=2

# Run waiter with custom number of waiters
./dining_philosophers --mode=2 --waiters=3

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
Configurable via `--asy=odd` or `--asy=even`. By default odd-indexed philosophers pick up the right fork first, breaking the circular wait condition. One philosopher is always able to make forward progress.

| Flag | Behaviour |
|---|---|
| `--asy=odd` | Odd-indexed philosophers (P1, P3) pick up right fork first (default) |
| `--asy=even` | Even-indexed philosophers (P0, P2, P4) pick up right fork first |

Expected output: All philosophers eat. No deadlock detected.

---

### mode=2 — Waiter (Semaphore Solution)
A `sem_t` counting semaphore limits how many philosophers can attempt to acquire forks simultaneously. The waiter count is configurable via `--waiters=N` (default: 4). Since at most N of five philosophers compete at once, at least one philosopher will always successfully acquire both forks, mathematically guaranteeing no deadlock.

Expected output: All philosophers eat. Semaphore never exceeds N concurrent. No deadlock detected.

---

## OS Primitives

| Primitive | Purpose |
|---|---|
| `pthread_create()` | Creates one philosopher thread per philosopher (5 total) |
| `pthread_join()` | Main thread waits for all philosopher threads to finish |
| `pthread_mutex_t` | One mutex per fork — enforces mutual exclusion |
| `pthread_mutex_lock()` | Philosopher claims a fork (blocks if already held) |
| `pthread_mutex_trylock()` | Non-blocking fork attempt — used for no infinite waiting |
| `pthread_mutex_unlock()` | Philosopher releases a fork when done eating |
| `sem_t` | Counting semaphore initialized to N for waiter solution |
| `sem_wait()` | Decrements semaphore — blocks if N philosophers already competing |
| `sem_post()` | Increments semaphore — signals next philosopher can attempt forks |

---

## Thread State Machine

Each philosopher thread continuously cycles through three states. Every state change updates the shared `state[i]` variable under mutex protection before changing fork ownership, preventing race conditions.

```
THINKING → HUNGRY → (wait for forks) → EATING → THINKING → ...
```

```c
sem_wait(&mutex);
state[id] = HUNGRY;             /* update state before fork attempt */
sem_post(&mutex);

pick_up_forks(id);              /* acquire forks */

sem_wait(&mutex);
state[id] = EATING;             /* confirm eating after forks acquired */
sem_post(&mutex);
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

### mode=1 — Asymmetric Algorithm (odd/even)
```c
if (asy_mode == ASY_ODD && (i % 2 != 0)) right_first = 1;
if (asy_mode == ASY_EVEN && (i % 2 == 0)) right_first = 1;

if (right_first) {
    pickup_forks_safe(i, right_fork(i), left_fork(i));
} else {
    pickup_forks_safe(i, left_fork(i), right_fork(i));
}
```

### mode=2 — Waiter Algorithm
```c
sem_wait(&waiter);                     /* block if N already competing */
pickup_forks_safe(i, left(i), right(i));
/* eat */
release_forks(i);
sem_post(&waiter);                     /* release seat at table */
```

### No Infinite Waiting — Trylock with Backoff
```c
while (pthread_mutex_trylock(&forks[fork_index]) != 0) {
    attempts++;
    if (attempts >= MAX_ATTEMPTS && first_fork != -1) {
        /* gave up — release first fork and retry */
        release_fork(first_fork);
        pthread_mutex_unlock(&forks[first_fork]);
        return 0;
    }
    usleep((rand() % 50 + 10) * 1000);   /* back off 10-60ms */
}
```

### Fork Conflict Detection
Every fork claim records the philosopher index in `fork_holder[]`. If a fork is claimed while already held a `FORK CONFLICT` warning is printed to stderr. This proves no two philosophers hold the same fork simultaneously.

```c
if (fork_holder[fork_index] != -1) {
    fprintf(stderr, "*** FORK CONFLICT: Fork %d already held ***\n", fork_index);
} else {
    fork_holder[fork_index] = (int)phil_id;
}
```

### Deadlock Detection
A monitor checks `state[i]` for all philosophers on every cycle. If all five are simultaneously HUNGRY and none are EATING, a deadlock is confirmed and reported.

---

## Input

The simulator takes no interactive input during runtime. All configuration is passed through command line arguments when launching the program.

| Argument | Values | Description |
|---|---|---|
| `--mode` | `0`, `1`, `2` | Sets the synchronization mode |
| `--asy` | `odd`, `even` | Asymmetric fork order for mode=1 (default: odd) |
| `--waiters` | `1` to `4` | Waiter semaphore count for mode=2 (default: 4) |
| `--steps` | Any positive integer | How many simulation cycles to run |
| `--help` | — | Prints usage instructions and exits |

If an unrecognized argument is passed, the program prints a friendly error message and exits cleanly.

---

## Output

The simulator produces a live state table updating every 0.5 seconds, a final statistics summary when the simulation ends, and a `simulation.log` file recording every event.

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

### No Infinite Waiting — Gave Up Event
```
P0 Aristotle gave up waiting for Fork 1 — releasing Fork 0 and retrying
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

### simulation.log — Status Tracking
Every state change and fork event is logged to `simulation.log` for full audit trail:

```
P0 Aristotle   -> THINKING
P0 Aristotle   -> HUNGRY
P0 Aristotle   -> FORK 0 CLAIMED
P0 Aristotle   -> FORK 1 CLAIMED
P0 Aristotle   -> EATING
P0 Aristotle   -> FORK 1 RELEASED
P0 Aristotle   -> FORK 0 RELEASED
P0 Aristotle   -> GAVE UP Fork 1 — releasing Fork 0 retrying
```

To verify no fork conflict:
```bash
grep "FORK 0" simulation.log   # should alternate CLAIMED then RELEASED
grep "GAVE UP" simulation.log  # shows no infinite waiting in action
```

---

## Fairness and Starvation

Fairness is measured by tracking meal counts per philosopher throughout the simulation. After every cycle the stats module compares each philosopher's meal count against the group average. If any philosopher's count lags significantly behind neighbors a starvation warning is printed to the terminal. Starvation is most likely in mode=1 under heavy load — watch P3 and P4 meal counts closely.

---

## File Structure

```
fork-or-starve/
│
├── Makefile
├── README.md
├── simulation.log      — generated at runtime — full event log
│
├── src/
│   ├── main.c          — entry point, CLI argument parsing
│   ├── philosopher.c   — thread logic, state machine, fork conflict detection
│   ├── philosopher.h
│   ├── forks.c         — left_fork() and right_fork() index helpers
│   ├── forks.h
│   ├── semaphore.c     — mutex and waiter semaphore initialization
│   ├── semaphore.h
│   ├── display.c       — terminal output, ANSI colors, 0.5s refresh
│   ├── display.h
│   ├── stats.c         — meal counts, starvation detection, print_stats
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
| Counting Semaphore (`sem_t`) | mode=2 waiter — limits concurrent access |
| Deadlock | All 4 conditions shown in mode=0 |
| Deadlock Prevention | mode=1 asymmetric and mode=2 waiter solutions |
| No Infinite Waiting | `pthread_mutex_trylock` with backoff and gave up retry |
| Fork Conflict Detection | `fork_holder[]` array tracks ownership — conflict logged to stderr |
| Status Tracking / Logging | Every event written to `simulation.log` |
| Random Sleep Times | `rand_sleep()` — 100ms to 600ms per state |
| Starvation Detection | Meal count comparison in statistics output |
| Race Conditions | Prevented via `state[i]` mutex protection |

---

## Known Limitations

- `sem_init()` deprecated on macOS — works correctly on course Linux VM
- Starvation possible in mode=1 under heavy load
- Simulation uses sleep-based timing rather than real concurrency scheduling
- Number of philosophers is fixed at 5 (configurable via `#define NUM_PHILS`)
- mode=0 stats do not print — threads deadlock and never finish — hit Ctrl+C to stop

---
