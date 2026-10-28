import std;
int main(int argc, char** argv) {
    try { throw std::logic_error("hosted"); }
    catch (const std::exception& e) { std::println("hosted ok: {}", e.what()); }
    std::println("hosted args: {}", argc);
    for (int i = 0; i < argc; ++i) std::println("  argv[{}] = {}", i, argv[i] ? argv[i] : "(null)");
    std::vector<std::string> v(argv, argv + argc);
    std::println("hosted vector: {}", v.size());
    return 0;
}
