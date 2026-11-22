// The entry of a program built over openkal-platform's carrier (mcpp SPEC-013
// §11). mcpp places what the program carries between `mcpp_carried_begin` and
// `mcpp_carried_end`, and its name at `mcpp_carried_name`:
//
//   nothing      the program is a package's runner
//                (`libexec/okrun/<os>-<arch>/okrun`): it is okrun's command
//                line (okrun_main, openkal-run 0.7.3), and so the loader an
//                image's start of an image file goes through;
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
    // A RUNNER NAMES ITSELF THE LOADER (0.31.3, openkal-image#46). With default
    // options it named none, and an image's start of an image file was refused:
    // on i686-openkal, whose runner this is, `posix_spawn` of the image itself
    // gave ENOEXEC, and a copy that called exec ended with 127.
    if (size == 0) return okrun_main(argc, argv);
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
