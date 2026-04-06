#include "RuntimeApplication.h"
#include <cstring>
#include <iostream>
#include <string>

namespace {
void printUsage(const char* argv0) {
    std::cout << "usage: " << argv0 << " [--world] <path-to-json>\n"
              << "  default mode loads a single level file\n"
              << "  --world loads a level group file\n";
}
}

int main(int argc, char** argv) {
    Runtime::LaunchOptions opts;
    std::string path;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--world") == 0) {
            opts.mode = Runtime::Mode::World;
        } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            std::cerr << "unknown flag: " << argv[i] << "\n";
            printUsage(argv[0]);
            return 1;
        } else {
            path = argv[i];
        }
    }

    if (path.empty()) {
        printUsage(argv[0]);
        return 1;
    }
    opts.path = path;

    Runtime::RuntimeApplication app;
    if (!app.initialize(opts)) return 1;
    app.run();
    return 0;
}
