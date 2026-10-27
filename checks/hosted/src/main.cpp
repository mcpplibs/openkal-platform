import std;
int main() {
    try { throw std::logic_error("hosted"); }
    catch (const std::exception& e) { std::println("hosted ok: {}", e.what()); }
    return 0;
}
