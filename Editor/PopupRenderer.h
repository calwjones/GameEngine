#pragma once
#include <string>

namespace Editor {

struct EditorContext;

class PopupRenderer {
public:
    enum class PendingAction { None, NewLevel, LoadLevel, CloseWindow, StartGroup };

private:
    EditorContext& ctx;
    bool m_showSaveAs = false;
    char m_saveAsBuf[256] = {0};
    bool m_showAbout = false;
    bool m_showLevelBrowser = false;
    PendingAction m_pendingAction = PendingAction::None;
    std::string m_pendingLoadPath;
    int m_pendingGroupIndex = -1;

public:
    explicit PopupRenderer(EditorContext& c) : ctx(c) {}
    void render();
    void saveLevel();
    void openSaveAsForCurrent();
    void openAbout() { m_showAbout = true; }
    void openLevelBrowser() { m_showLevelBrowser = true; }
    void requestCloseWindow() { m_pendingAction = PendingAction::CloseWindow; }
    void requestNewLevel() { m_pendingAction = PendingAction::NewLevel; }
    void requestLoadLevel(std::string path) {
        m_pendingAction = PendingAction::LoadLevel;
        m_pendingLoadPath = std::move(path);
    }
    void requestStartGroup(int idx) {
        m_pendingAction = PendingAction::StartGroup;
        m_pendingGroupIndex = idx;
    }

private:
    void showUnsavedChangesDialog();
};

}
