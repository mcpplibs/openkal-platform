// A DOS program: its arguments sorted, an exception across a frame, the environment, a deep
// stack, a status. The recursion uses 512 KiB (64 frames of 8 KiB): the row's stack is 1 MiB
// (0.29.1, `-z stack-size'), which openkal-dos 0.1.1's stub reads from PT_GNU_STACK.
import std;

#include <spawn.h>
#include <sys/wait.h>

extern "C" char** environ;

int deep(int n) {
    volatile char frame[8192];
    frame[0] = static_cast<char>(n);
    return n == 0 ? frame[0] : deep(n - 1) + frame[0];
}

// A directory made, a file written, renamed and listed by its whole name, and removed (0.29.2):
// on the release profile openkal-uefi 0.11.1 listed `g1.' for `g1.txt' and renamed nothing, and
// under DOS a listing after 32 changes of working directory found nothing (openkal-musl 0.29.8).
bool files() {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::remove_all("okp", ec);
    if (!fs::create_directory("okp", ec)) return false;
    { std::ofstream f("okp/f1.txt"); f << "x\n"; }
    fs::rename("okp/f1.txt", "okp/g1.txt", ec);
    if (ec) return false;
    const auto here = fs::current_path(ec);
    for (int i = 0; i < 48 && !ec; ++i) { fs::current_path("okp", ec); if (!ec) fs::current_path(here, ec); }
    if (ec) return false;
    std::vector<std::string> names;
    for (const auto& e : fs::directory_iterator("okp", ec)) {
        auto n = e.path().filename().string();
        for (auto& c : n) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        names.push_back(n);
    }
    fs::remove_all("okp", ec);
    return names == std::vector<std::string>{"g1.txt"};
}

// A START OF A PROGRAM (0.32.0): the program starts itself by posix_spawn with `child', which ends
// with 3, and waitpid reads the status; the row's implementation provides openkal.process, a
// started program run to its end before the start returns (openkal clause 7.19).
int starts(const char* self) {
    pid_t pid;
    char* args[] = {const_cast<char*>(self), const_cast<char*>("child"), nullptr};
    if (posix_spawn(&pid, self, nullptr, nullptr, args, environ) != 0) return -1;
    int status = 0;
    if (waitpid(pid, &status, 0) != pid) return -2;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -3;
}

int main(int argc, char** argv) {
    if (argc >= 2 && std::string_view(argv[1]) == "child") return 3;
    std::vector<std::string> v(argv + 1, argv + argc);
    std::ranges::sort(v);
    std::println("dos ok: {}", v);
    try { throw std::runtime_error("across a frame"); }
    catch (const std::exception& e) { std::println("caught: {}", e.what()); }
    std::println("PATH set: {}", std::getenv("PATH") != nullptr);
    std::println("deep: {}", deep(64));
    std::println("files: {}", files());
    std::println("starts: child {}", starts(argv[0]));
    return 7;
}
