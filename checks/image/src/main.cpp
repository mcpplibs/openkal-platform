import std;
int main(int argc, char** argv) {
    try { throw std::runtime_error("image"); }
    catch (const std::exception& e) { std::println("{} ok: {} args", e.what(), argc); }
    return 0;
}
