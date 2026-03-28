#include "Editor/EditorApplication.h"
#include <filesystem>
#include <iostream>

int main(int argc, char *argv[]) {
  // pin cwd to exe dir so asset paths resolve when launched from Finder
  if (argc > 0 && argv[0] != nullptr) {
    std::error_code ec;
    std::filesystem::path exePath = std::filesystem::absolute(argv[0], ec);
    if (!ec) {
      std::filesystem::current_path(exePath.parent_path(), ec);
    }
  }

  Editor::EditorApplication editor;
  if (!editor.initialize(1280, 720)) {
    std::cerr << "Failed to initialise editor" << std::endl;
    return 1;
  }
  editor.run();
  return 0;
}
