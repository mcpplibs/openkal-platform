// A System V program: its arguments sorted, an exception across a frame, the environment, a
// directory made, a file written, renamed, listed by its whole name and removed, a status. The
// directory holds more names than one reading of it returns (a listing in parts), and the
// working directory is changed and restored, as on the UEFI and DOS rows.
import std;

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
    for (int i = 0; i < 300; ++i) { std::ofstream f(std::format("okp/a-file-with-a-long-name-{:03}.txt", i)); f << i; }
    std::size_t count = 0;
    bool renamed = false;
    for (const auto& e : fs::directory_iterator("okp", ec)) {
        ++count;
        renamed = renamed || e.path().filename() == "g1.txt";
    }
    const auto size = fs::file_size("okp/g1.txt", ec);
    const auto removed = fs::remove_all("okp", ec);
    return !ec && renamed && count == 301 && size == 2 && removed == 302;
}

int main(int argc, char** argv) {
    std::vector<std::string> v(argv + 1, argv + argc);
    std::ranges::sort(v);
    std::println("svr4 ok: {}", v);
    try { throw std::runtime_error("across a frame"); }
    catch (const std::exception& e) { std::println("caught: {}", e.what()); }
    std::println("PATH set: {}", std::getenv("PATH") != nullptr);
    std::println("here: {}", std::filesystem::current_path().is_absolute());
    std::println("files: {}", files());
    return 7;
}
