// What an ok image is given by this platform, checked inside one: the C library
// and the C++ run-time in the image (exceptions, thread-local storage, the
// standard library's containers and formatting), its arguments and environment
// through the loader, and a thread.
import std;

namespace {

thread_local int tls = 7;

int fail(std::string_view what) {
    std::println("FAIL: {}", what);
    return 1;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 1 || argv[0] == nullptr) return fail("no argv[0]");
    try {
        throw std::runtime_error("thrown");
    } catch (const std::exception& e) {
        if (std::string_view(e.what()) != "thrown") return fail("the exception's message");
    }
    std::vector<int> v{3, 1, 2};
    std::ranges::sort(v);
    if (std::format("{}", v) != "[1, 2, 3]") return fail("format of a vector");
    int seen = 0;
    std::thread t([&] { tls = 9; seen = tls; });
    t.join();
    if (seen != 9 || tls != 7) return fail("thread-local storage per thread");
    if (std::getenv("PATH") == nullptr) return fail("the environment through the loader");
    std::println("image ok: {} ({} args)", argv[0], argc);
    return 0;
}
