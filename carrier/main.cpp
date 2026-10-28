// The entry of a program built over openkal-platform's carrier (mcpp SPEC-013
// §11). mcpp places what the program carries between `mcpp_carried_begin` and
// `mcpp_carried_end`, and its name at `mcpp_carried_name`:
//
//   nothing      the program is a package's runner
//                (`libexec/okrun/<os>-<arch>/okrun`): it runs the image, or the
//                okb, its first argument names, as okrun does;
//   a zip        the images of a program (a single file): the entry and the
//                images it needs run from memory, nothing written to disk;
//   an image     the image runs from memory, as its own program (M3).
#include <okrun.h>
#include <cstdio>
#include <cstring>

extern "C" const unsigned char mcpp_carried_begin[], mcpp_carried_end[];
extern "C" const char mcpp_carried_name[];

int main(int argc, char** argv) {
    char why[512] = {};
    okrun_options options{};
    const auto size = static_cast<unsigned long long>(mcpp_carried_end - mcpp_carried_begin);
    const auto args = const_cast<const char* const*>(argv);
    if (size == 0) {
        if (argc < 2) {
            std::fprintf(stderr, "usage: %s <image|okb> [arguments...]\n", argv[0]);
            return 2;
        }
        okrun_run_file(argv[1], argc - 1, args + 1, &options, why, sizeof why);
        std::fprintf(stderr, "%s: %s\n", argv[1], why);
        return 127;
    }
    options.carried = 1;   // the program is the image's name, and what starts it again
    if (size >= 4 && std::memcmp(mcpp_carried_begin, "PK\x03\x04", 4) == 0)
        okrun_run_okb_bytes(mcpp_carried_begin, size, mcpp_carried_name, argc, args, &options,
                            why, sizeof why);
    else
        okrun_run_bytes(mcpp_carried_begin, size, mcpp_carried_name, argc, args, &options,
                        why, sizeof why);
    std::fprintf(stderr, "%s: %s\n", mcpp_carried_name, why);
    return 127;
}
