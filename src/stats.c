#include <stdio.h>
#include "philosopher.h"

#define YELLOW "\033[93m"
#define RESET "\033[0m"

void meal_record(long i) {
	meals[i]++;
	printf("%sP%ld %s have eaten %d meal(s)%s\n", YELLOW, i, phil_names[i], meals[i], RESET);
}
