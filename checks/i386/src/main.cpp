// An i386 image: its arguments sorted, an exception across a frame, the environment, a status.
import std;

int main(int argc, char** argv) {
    std::vector<std::string> v(argv + 1, argv + argc);
    std::ranges::sort(v);
    std::println("i386 ok: {}", v);
    try { throw std::runtime_error("across a frame"); }
    catch (const std::exception& e) { std::println("caught: {}", e.what()); }
    std::println("PATH set: {}", std::getenv("PATH") != nullptr);
    return 7;
}
