// entry point — literally just pins cwd + spins up the editor
// (no separate "game" binary, editor IS the game rn)
#include "Editor/EditorApplication.h"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    // pin cwd to wherever the exe lives so "assets/..." paths just work.
    // without this, double-clicking from Finder makes cwd = home dir and every
    // texture/sound lookup silently fails. spent way too long on this bug lol
    if (argc > 0 && argv[0] != nullptr) {
        std::error_code ec;
        std::filesystem::path exePath = std::filesystem::absolute(argv[0], ec);
        if (!ec) {
            std::filesystem::current_path(exePath.parent_path(), ec);
        }
    }

    // editor owns EVERYTHING — window, subsystems, the lot.
    // initialize() returns false if SFML/ImGui setup blows up
    Editor::EditorApplication editor;
    if (!editor.initialize(1280, 720)) {
        std::cerr << "Failed to initialise editor" << std::endl;
        return 1;
    }
    editor.run();   // blocks till user closes the window
    return 0;
}
