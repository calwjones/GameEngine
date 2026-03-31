#include "PopupRenderer.h"
#include "EditorContext.h"
#include "LevelIO.h"
#include "PlaySessionController.h"
#include <imgui.h>
#include <cstdio>

namespace Editor {

void PopupRenderer::saveLevel() {
    if (ctx.levelIO->currentPath().empty()) {
        openSaveAsForCurrent();
        return;
    }
    ctx.levelIO->saveLevel();
}

void PopupRenderer::openSaveAsForCurrent() {
    std::string defaultName = "my_level.json";
    if (!ctx.levelIO->currentPath().empty()) {
        const auto& p = ctx.levelIO->currentPath();
        auto pos = p.find_last_of('/');
        defaultName = (pos != std::string::npos) ? p.substr(pos + 1) : p;
    }
    snprintf(m_saveAsBuf, sizeof(m_saveAsBuf), "%s", defaultName.c_str());
    m_showSaveAs = true;
}

void PopupRenderer::render() {
    showUnsavedChangesDialog();

    if (m_showSaveAs) {
        ImGui::OpenPopup("Save Level As");
        m_showSaveAs = false;
    }
    if (ImGui::BeginPopupModal("Save Level As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Filename:");
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##saveas", m_saveAsBuf, sizeof(m_saveAsBuf));
        ImGui::TextDisabled("Saved to assets/levels/");

        std::string name(m_saveAsBuf);
        bool nameValid = !name.empty();
        if (nameValid) {
            for (char c : name) {
                if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
                    c == '"' || c == '<' || c == '>' || c == '|') {
                    nameValid = false;
                    break;
                }
            }
        }
        if (!nameValid && !name.empty())
            ImGui::TextColored(ImVec4(1.f, 0.4f, 0.3f, 1.f), "Invalid filename characters");

        ImGui::Spacing();
        if (!nameValid) ImGui::BeginDisabled();
        if (ImGui::Button("Save", ImVec2(140, 0))) {
            if (name.size() < 5 || name.substr(name.size() - 5) != ".json")
                name += ".json";
            ctx.levelIO->saveLevelAs(name);
            ImGui::CloseCurrentPopup();
        }
        if (!nameValid) ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (m_showLevelBrowser) {
        ImGui::OpenPopup("Level Browser");
        m_showLevelBrowser = false;
    }
    ImGui::SetNextWindowSize(ImVec2(500, 560), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Level Browser", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::TextColored(ImVec4(0.30f, 0.78f, 0.70f, 1.f), "Level Browser");
        ImGui::TextDisabled("Start a world or open a single level.");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.55f, 0.85f, 1.0f, 1.f), "Worlds");
        ImGui::TextDisabled("Chained levels with lives and a running coin total.");
        ImGui::Spacing();

        ImGui::BeginChild("##WorldList", ImVec2(0, 180), true);
        if (ctx.levelIO->groups().empty()) {
            ImGui::TextDisabled("No worlds found in assets/levels/groups/");
        } else {
            for (int i = 0; i < (int)ctx.levelIO->groups().size(); ++i) {
                auto& g = ctx.levelIO->groups()[i];
                ImGui::PushID(i);

                ImGui::TextColored(ImVec4(1.f, 0.92f, 0.65f, 1.f), "%s", g.name.c_str());
                ImGui::SameLine();
                ImGui::TextDisabled("(%d levels - %d lives)", (int)g.levels.size(), g.lives);

                for (size_t li = 0; li < g.levels.size(); ++li) {
                    if (li == 0)
                        ImGui::TextDisabled("  1. %s  (start)", g.levels[li].c_str());
                    else
                        ImGui::TextDisabled("  %d. %s  (locked)", (int)li + 1, g.levels[li].c_str());
                }

                if (ImGui::Button("Start World", ImVec2(140, 0))) {
                    if (ctx.dirty) {
                        requestStartGroup(i);
                    } else {
                        ctx.playSession->startGroup(i);
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                ImGui::PopID();
            }
        }
        ImGui::EndChild();

        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.f), "Single Levels");
        ImGui::TextDisabled("Open any level for editing or solo play (no lives).");
        ImGui::Spacing();

        if (ctx.levelIO->levelFiles().empty()) {
            ImGui::TextDisabled("No levels found in assets/levels/");
        } else {
            ImGui::BeginChild("##LevelList", ImVec2(0, -40), true);
            for (auto& file : ctx.levelIO->levelFiles()) {
                std::string display = file;
                auto slash = display.find_last_of("/\\");
                if (slash != std::string::npos) display = display.substr(slash + 1);

                bool current = (file == ctx.levelIO->currentPath());
                if (current) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.42f, 0.38f, 1.f));
                if (ImGui::Button(display.c_str(), ImVec2(-1, 28))) {
                    ctx.playSession->endGroupSession();
                    if (ctx.dirty) {
                        requestLoadLevel(file);
                    } else {
                        ctx.levelIO->loadLevel(file);
                    }
                    ImGui::CloseCurrentPopup();
                }
                if (current) ImGui::PopStyleColor();
                if (current) {
                    ImGui::SameLine();
                    ImGui::TextDisabled("(current)");
                }
            }
            ImGui::EndChild();
        }

        ImGui::Spacing();
        if (ImGui::Button("Refresh", ImVec2(120, 0))) {
            ctx.levelIO->scanLevelFiles();
            ctx.levelIO->scanLevelGroups();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (m_showAbout) {
        ImGui::OpenPopup("About");
        m_showAbout = false;
    }
    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.30f, 0.78f, 0.70f, 1.f), "2D Game Engine Editor");
        ImGui::TextDisabled("v1.0.0");
        ImGui::Separator();
        ImGui::Text("C++17 / SFML 2.x / Dear ImGui");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(200, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void PopupRenderer::showUnsavedChangesDialog() {
    if (m_pendingAction == PendingAction::None) return;

    ImGui::OpenPopup("Unsaved Changes");
    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You have unsaved changes. What would you like to do?");
        ImGui::Spacing();

        auto runPending = [&]() {
            auto action = m_pendingAction;
            auto path = m_pendingLoadPath;
            int groupIdx = m_pendingGroupIndex;
            m_pendingAction = PendingAction::None;
            m_pendingLoadPath.clear();
            m_pendingGroupIndex = -1;

            if (action == PendingAction::NewLevel) {
                ctx.levelIO->resetToNewLevel();
            } else if (action == PendingAction::LoadLevel) {
                ctx.levelIO->loadLevel(path);
            } else if (action == PendingAction::StartGroup) {
                ctx.playSession->startGroup(groupIdx);
            } else if (action == PendingAction::CloseWindow) {
                ctx.window.close();
            }
        };

        if (ImGui::Button("Save", ImVec2(120, 0))) {
            saveLevel();
            ImGui::CloseCurrentPopup();
            runPending();
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            ctx.dirty = false;
            runPending();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            m_pendingAction = PendingAction::None;
            m_pendingLoadPath.clear();
            m_pendingGroupIndex = -1;
        }
        ImGui::EndPopup();
    }
}

}
