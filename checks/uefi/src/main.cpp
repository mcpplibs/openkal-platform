// The C++ a UEFI application is written in: the standard library's text, ranges and containers
// over openkal-musl and openkal-llvm-runtime, its arguments as the runner passed them (0.29.0: the
// Shell's, an argument with a space whole), and its status returned to the machine that ran it.
import std;

int main(int argc, char** argv) {
    std::vector<int> v{3, 1, 2};
    std::ranges::sort(v);
    std::vector<std::string_view> args(argv + 1, argv + argc);
    std::println("uefi ok: {} {}", v, args);
    return 42;
}
