/*
 * File: display.c
 * Purpose: Prints each philosopher's current state to the terminal.
 *          Includes live table that refreshes every 0.5 seconds.
 */

#include <stdio.h>
#include <unistd.h>
#include "philosopher.h"
#include "forks.h"

#define GREEN   "\033[92m"
#define RED     "\033[91m"
#define BLUE    "\033[94m"
#define CYAN    "\033[96m"
#define BOLD    "\033[1m"
#define WHITE   "\033[97m"
#define RESET   "\033[0m"

/*
 * Function: print_state
 * Purpose: Prints one philosopher's current state with a color.
 * Params:
 *   i     - philosopher number
 *   state - current state of the philosopher
 * Returns: void
 */
void print_state(long i, int state) {
    if (state == THINKING) {
        printf("%sP%ld %s is THINKING%s\n", BLUE, i, phil_names[i], RESET);
    }
    else if (state == HUNGRY) {
        printf("%sP%ld %s is HUNGRY%s\n", RED, i, phil_names[i], RESET);
    }
    else if (state == EATING) {
        printf("%sP%ld %s is EATING%s\n", GREEN, i, phil_names[i], RESET);
    }
}

/*
 * Function: clear_screen
 * Purpose: Clears terminal for live table refresh
 *          Only clears if running in a real terminal — not when redirected to file
 * Returns: void
 */
void clear_screen() {
    if (isatty(fileno(stdout))) {   /* only clear if running in real terminal */
        printf("\033[2J");
        printf("\033[H");
    }
    fflush(stdout);
}

/*
 * Function: print_table
 * Purpose:  Prints live state table of all philosophers
 *           Shows state, forks held, and meal count
 *           Shows deadlock alert if all philosophers stuck in HUNGRY
 * Returns:  void
 */
void print_table() {
    clear_screen();

    printf(BOLD CYAN "╔══════════════════════════════════════════════════════════╗\n" RESET);
    printf(BOLD CYAN "║        fork-or-starve  v1.0                              ║\n" RESET);
    printf(BOLD CYAN "╚══════════════════════════════════════════════════════════╝\n\n" RESET);

    printf(BOLD WHITE "  %-4s %-12s %-10s %-12s %-12s %s\n" RESET,
           "ID", "NAME", "STATE", "LEFT FORK", "RIGHT FORK", "MEALS");
    printf(CYAN "  ──────────────────────────────────────────────────────────\n" RESET);

    for (int i = 0; i < NUM_PHILS; i++) {
        char *color, *state_str;
        if (state[i] == EATING)       { color = GREEN; state_str = "EATING";   }
        else if (state[i] == HUNGRY)  { color = RED;   state_str = "HUNGRY";   }
        else                          { color = BLUE;  state_str = "THINKING"; }

        char left_str[16], right_str[16];
        sprintf(left_str,  "F%d: %s", left_fork(i),  (state[i] == EATING) ? "HELD" : "free");
        sprintf(right_str, "F%d: %s", right_fork(i), (state[i] == EATING) ? "HELD" : "free");

        printf("  P%d  %-12s %s%-10s%s %-12s %-12s %d\n",
               i, phil_names[i],
               color, state_str, RESET,
               left_str, right_str, meals[i]);
    }

    printf(CYAN "  ──────────────────────────────────────────────────────────\n\n" RESET);

    /* deadlock alert — check if all philosophers stuck in HUNGRY */
    if (mode == 0) {
        int all_hungry = 1;
        for (int i = 0; i < NUM_PHILS; i++) {
            if (state[i] != HUNGRY) { all_hungry = 0; break; }
        }
        if (all_hungry) {
            printf(RED BOLD "  *** DEADLOCK DETECTED — all philosophers waiting, none eating ***\n" RESET);
        }
    }

    fflush(stdout);
}

/*
 * Function: display_thread
 * Purpose:  Runs as a separate thread refreshing the table every 0.5 seconds
 * Params:   arg — unused
 * Returns:  NULL
 */
void *display_thread(void *arg) {
    while (1) {
        print_table();
        usleep(500000);   /* refresh every 0.5 seconds */
    }
    return NULL;
}
