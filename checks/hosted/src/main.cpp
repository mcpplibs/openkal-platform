import std;
// The hosted arrangement's program: an exception crosses the C++ run-time,
// and the arguments it was started with arrive.
int main(int argc, char** argv) {
    try { throw std::logic_error("hosted"); }
    catch (const std::exception& e) { std::println("hosted ok: {}", e.what()); }
    std::vector<std::string> args(argv, argv + argc);
    std::println("hosted args: {}{}", args.size(), args.size() > 1 ? " " + args.back() : std::string());
    return 0;
}
