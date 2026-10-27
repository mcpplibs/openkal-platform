// The entry of the program `mcpp pack --mode self-contained` builds for an ok
// image (M3): it runs the image it carries through libokrun. mcpp places the
// image's bytes between `mcpp_carried_begin` and `mcpp_carried_end` and its
// name at `mcpp_carried_name` (SPEC-013 §4.7).
#include <okrun.h>
#include <cstdio>

extern "C" const unsigned char mcpp_carried_begin[], mcpp_carried_end[];
extern "C" const char mcpp_carried_name[];

int main(int argc, char** argv) {
    char why[512];
    okrun_options options{};
    options.carried = 1;   // the program is the image's name, and what starts it again
    okrun_run_bytes(mcpp_carried_begin,
                    static_cast<unsigned long long>(mcpp_carried_end - mcpp_carried_begin),
                    mcpp_carried_name, argc, const_cast<const char* const*>(argv), &options,
                    why, sizeof why);
    std::fprintf(stderr, "%s: %s\n", mcpp_carried_name, why);
    return 127;
}
