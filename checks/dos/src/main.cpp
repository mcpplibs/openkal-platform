// A DOS program: its arguments sorted, an exception across a frame, the environment, a deep
// stack, a status. The recursion uses 512 KiB (64 frames of 8 KiB): the row's stack is 1 MiB
// (0.29.1, `-z stack-size'), which openkal-dos 0.1.1's stub reads from PT_GNU_STACK.
import std;

int deep(int n) {
    volatile char frame[8192];
    frame[0] = static_cast<char>(n);
    return n == 0 ? frame[0] : deep(n - 1) + frame[0];
}

int main(int argc, char** argv) {
    std::vector<std::string> v(argv + 1, argv + argc);
    std::ranges::sort(v);
    std::println("dos ok: {}", v);
    try { throw std::runtime_error("across a frame"); }
    catch (const std::exception& e) { std::println("caught: {}", e.what()); }
    std::println("PATH set: {}", std::getenv("PATH") != nullptr);
    std::println("deep: {}", deep(64));
    return 7;
}
