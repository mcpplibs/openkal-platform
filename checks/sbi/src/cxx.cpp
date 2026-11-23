// C++23 on the machine: `import std', containers and ranges, an exception thrown across frames and
// caught with a destructor run on the way, and a status the machine ends with.
import std;

namespace {
int unwound = 0;
struct guard { ~guard() { ++unwound; } };
[[gnu::noinline]] void thrower(int depth) {
    guard g;
    if (depth == 0) throw std::runtime_error("across frames");
    thrower(depth - 1);
}
}  // namespace

int main() {
    std::vector<std::string> v{"zeta", "alpha", "mu"};
    std::ranges::sort(v);
    std::println("sbi cxx: {}", v);
    try { thrower(2); } catch (const std::exception& e) { std::println("caught: {}, {} unwound", e.what(), unwound); }
    return 9;
}
