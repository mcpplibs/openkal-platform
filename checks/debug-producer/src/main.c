__attribute__((visibility("default")))
int ok_main(void) {
    volatile int answer = 40;
    answer += 2;
    return answer == 42 ? 0 : 1;
}
