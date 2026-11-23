/* Started with no argument, the image starts itself with `child' by posix_spawn and
 * writes the child's status; started with `child', it ends with 3. */
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

extern char** environ;

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "child") == 0) return 3;
    pid_t pid;
    char* args[] = { argv[0], "child", NULL };
    const int rc = posix_spawn(&pid, argv[0], NULL, NULL, args, environ);
    if (rc != 0) { printf("starts: posix_spawn %d\n", rc); return 1; }
    int status = 0;
    if (waitpid(pid, &status, 0) != pid) { printf("starts: waitpid\n"); return 1; }
    printf("starts: child %d\n", WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    return 0;
}
