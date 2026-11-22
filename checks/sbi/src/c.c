/* C on the machine: the console, the allocator, the clock, and a status the machine ends with. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(void) {
    char* p = malloc(1 << 20);
    if (p) memset(p, 1, 1 << 20);
    struct timespec a, b;
    clock_gettime(CLOCK_MONOTONIC, &a);
    clock_gettime(CLOCK_MONOTONIC, &b);
    printf("sbi c: %s, the clock %s\n", p ? "a mebibyte allocated" : "no memory",
           (b.tv_sec > a.tv_sec || b.tv_nsec >= a.tv_nsec) ? "advances" : "runs back");
    free(p);
    return 7;
}
