// The C++ a UEFI application is written in: the standard library's text, ranges and containers
// over openkal-musl and openkal-llvm-runtime, and its status returned to the machine that ran it.
import std;

int main(int argc, char** argv) {
    std::vector<int> v{3, 1, 2};
    std::ranges::sort(v);
    std::println("uefi ok: {} {}", v, argc);
    return 42;
}
