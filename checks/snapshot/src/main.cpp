// A program whose initialisers build strings, a map and a vector, and whose
// main then uses the standard streams, an exception, threads and their
// thread-local objects --- the same at a start from a snapshot of its
// initialisers as at one that runs them.
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {
std::string g_name = std::string("snapshot-") + std::to_string(42);
std::map<std::string, int> g_table = [] {
    std::map<std::string, int> m;
    for (int i = 0; i < 5000; ++i) m.emplace("key" + std::to_string(i), i * 3);
    return m;
}();
std::vector<std::unique_ptr<std::string>> g_strings = [] {
    std::vector<std::unique_ptr<std::string>> v;
    for (int i = 0; i < 3000; ++i) v.push_back(std::make_unique<std::string>(std::string(i % 200 + 1, 'a' + i % 26)));
    return v;
}();
// Read once while the initialisers run: a snapshot records the name, and a
// start whose value differs does not use it.
const char* g_named = std::getenv("SNAPSHOT_CHECK_NAME");
struct counted {
    std::string s = "thread";
    ~counted() { ++destroyed; }
    static inline std::atomic<int> destroyed{0};
};
thread_local counted t_counted;

int failures = 0;
void check(bool held, const char* what) { if (!held) { std::printf("FAIL: %s\n", what); ++failures; } }
}  // namespace

int main(int argc, char** argv) {
    check(g_name == "snapshot-42", "the global string");
    check(g_table.size() == 5000 && g_table.at("key4999") == 4999 * 3, "the map");
    bool strings = g_strings.size() == 3000;
    for (int i = 0; strings && i < 3000; ++i) strings = *g_strings[i] == std::string(i % 200 + 1, 'a' + i % 26);
    check(strings, "the vector of strings");
    static const std::string local = std::string("local ") + (argc > 1 ? argv[1] : "none");
    std::cout << "iostream: " << g_name << " " << local << std::endl;
    try { throw std::runtime_error("thrown"); } catch (const std::exception& e) { check(std::string(e.what()) == "thrown", "an exception is caught"); }
    std::vector<std::thread> threads;
    std::atomic<int> sum{0};
    for (int k = 0; k < 4; ++k) threads.emplace_back([&] { sum += static_cast<int>(t_counted.s.size()); });
    for (auto& t : threads) t.join();
    check(sum == 24, "the threads' thread-local objects");
    check(counted::destroyed >= 4, "and their destructors ran");
    std::printf("named: %s\n", g_named ? g_named : "(absent)");
    g_table.clear();
    g_strings.clear();
    std::printf("-- failures: %d --\n", failures);
    return failures != 0;
}
