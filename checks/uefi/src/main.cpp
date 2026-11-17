// The C++ a UEFI application is written in: the standard library's text, ranges and containers
// over openkal-musl and openkal-llvm-runtime, its arguments as the runner passed them (0.29.0: the
// Shell's, an argument with a space whole; 0.29.1: runs of blanks inside one kept), and its status
// returned to the machine that ran it.
import std;

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

int main(int argc, char** argv) {
    std::vector<int> v{3, 1, 2};
    std::ranges::sort(v);
    std::vector<std::string_view> args(argv + 1, argv + argc);
    std::println("uefi ok: {} {}", v, args);
    std::println("files: {}", files());
    return 42;
}
