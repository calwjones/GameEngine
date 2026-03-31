#include "MenuBarRenderer.h"
#include "EditorContext.h"
#include "SelectionController.h"
#include "LevelIO.h"
#include "PlaySessionController.h"
#include "EntityOps.h"
#include "PopupRenderer.h"
#include <imgui.h>
#include <string>

namespace Editor {

void MenuBarRenderer::render(float dt) {
    if (!ImGui::BeginMainMenuBar()) return;

    bool isPlaying = ctx.playSession->isPlaying();

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Level", "Ctrl+N", false, !isPlaying)) {
            if (ctx.dirty) {
                ctx.popups->requestNewLevel();
            } else {
                ctx.levelIO->resetToNewLevel();
            }
        }

        if (ImGui::BeginMenu("Load Level")) {
            if (ctx.levelIO->levelFiles().empty()) {
                ImGui::TextDisabled("No levels found in assets/levels/");
            } else {
                for (auto& file : ctx.levelIO->levelFiles()) {
                    std::string display = file;
                    if (file == ctx.levelIO->currentPath()) display += "  (current)";
                    if (ImGui::MenuItem(display.c_str())) {
                        if (ctx.dirty) {
                            ctx.popups->requestLoadLevel(file);
                        } else {
                            ctx.levelIO->loadLevel(file);
                        }
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Refresh")) ctx.levelIO->scanLevelFiles();
            ImGui::EndMenu();
        }

        ImGui::Separator();

        bool canSave = !ctx.levelIO->currentPath().empty() && !isPlaying;
        if (ImGui::MenuItem("Save", "Ctrl+S", false, canSave)) ctx.popups->saveLevel();
        if (ImGui::MenuItem("Save As...", nullptr, false, !isPlaying)) {
            ctx.popups->openSaveAsForCurrent();
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4")) ctx.window.close();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, ctx.history.canUndo() && ctx.state == Engine::GameState::MENU)) {
            std::string desc = ctx.history.undoDescription();
            ctx.history.undo();
            ctx.selection->validate();
            ctx.selection->findPlayer();
            ctx.markDirty();
            ctx.setStatus("Undo: " + desc);
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, ctx.history.canRedo() && ctx.state == Engine::GameState::MENU)) {
            ctx.history.redo();
            ctx.selection->validate();
            ctx.selection->findPlayer();
            ctx.markDirty();
            ctx.setStatus("Redo: " + ctx.history.undoDescription());
        }
        ImGui::Separator();

        bool hasSelected = ctx.selection->current() != nullptr && !isPlaying;
        if (ImGui::MenuItem("Add Entity", nullptr, false, !isPlaying)) ctx.entityOps->addEntity();
        if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, hasSelected)) ctx.entityOps->duplicateEntity();
        if (ImGui::MenuItem("Delete", "Del", false, !ctx.selection->all().empty() && ctx.state == Engine::GameState::MENU)) ctx.entityOps->deleteSelected();
        ImGui::Separator();
        if (ImGui::MenuItem("Deselect", "Esc", false, !ctx.selection->all().empty())) {
            ctx.selection->clear();
            ctx.viewport.cancelDrag();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Scene Panel", nullptr, &ctx.scenePanel.isVisible());
        ImGui::MenuItem("Properties Panel", nullptr, &ctx.propsPanel.isVisible());
        ImGui::MenuItem("Game Viewport", nullptr, &ctx.viewport.isVisible());
        ImGui::MenuItem("Entity Palette", nullptr, &ctx.palette.isVisible());
        ImGui::Separator();
        ImGui::MenuItem("Grid", nullptr, &ctx.viewport.gridEnabled());
        if (ImGui::BeginMenu("Grid Size")) {
            float sizes[] = {8, 16, 32, 64};
            for (float s : sizes) {
                char label[16];
                snprintf(label, sizeof(label), "%.0fpx", s);
                if (ImGui::MenuItem(label, nullptr, ctx.viewport.gridSize() == s))
                    ctx.viewport.gridSize() = s;
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Reset Viewport")) ctx.viewport.resetView();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        if (ImGui::BeginMenu("Shortcuts")) {
            ImGui::TextDisabled("Ctrl+N       New Level");
            ImGui::TextDisabled("Ctrl+S       Save Level");
            ImGui::TextDisabled("Ctrl+Z       Undo");
            ImGui::TextDisabled("Ctrl+Shift+Z Redo");
            ImGui::TextDisabled("Ctrl+D       Duplicate Entity");
            ImGui::TextDisabled("Ctrl+Click   Multi-select");
            ImGui::TextDisabled("Del          Delete Entity");
            ImGui::TextDisabled("Esc          Deselect / Pause");
            ImGui::TextDisabled("Space        Play");
            ImGui::TextDisabled("G            Toggle grid");
            ImGui::TextDisabled("F            Focus on selection");
            ImGui::TextDisabled("Middle-drag  Pan viewport");
            ImGui::TextDisabled("Scroll       Zoom viewport");
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("About")) ctx.popups->openAbout();
        ImGui::EndMenu();
    }

    float rightEdge = ImGui::GetWindowWidth() - 20;
    if (ctx.statusTimer > 0) {
        float alpha = ctx.statusTimer < 0.5f ? ctx.statusTimer * 2.f : 1.f;
        ImGui::SameLine(rightEdge - ImGui::CalcTextSize(ctx.statusMsg.c_str()).x);
        ImGui::TextColored(ImVec4(0.30f, 0.78f, 0.70f, alpha), "%s", ctx.statusMsg.c_str());
    } else {
        m_fpsAccum += dt;
        m_fpsFrames++;
        if (m_fpsAccum >= 0.5f) {
            m_displayFps = m_fpsAccum > 0 ? (int)(m_fpsFrames / m_fpsAccum) : 60;
            m_fpsAccum = 0.f;
            m_fpsFrames = 0;
        }
        std::string txt = "FPS: " + std::to_string(m_displayFps);
        ImGui::SameLine(rightEdge - ImGui::CalcTextSize(txt.c_str()).x);
        ImGui::Text("%s", txt.c_str());
    }

    ImGui::EndMainMenuBar();
}

}
