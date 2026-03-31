#include "ToolbarRenderer.h"
#include "EditorContext.h"
#include "SelectionController.h"
#include "LevelIO.h"
#include "PlaySessionController.h"
#include "EntityOps.h"
#include "PopupRenderer.h"
#include <imgui.h>
#include <algorithm>
#include <string>

namespace Editor {

void ToolbarRenderer::render() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowPos(ImVec2(0, 19));
    ImGui::SetNextWindowSize(ImVec2((float)ctx.window.getSize().x, 40));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 5));

    bool isPlaying = ctx.playSession->isPlaying();

    if (ImGui::Begin("Toolbar", nullptr, flags)) {
        bool canPlay = ctx.viewport.isVisible() && ctx.game.getEntityManager().getEntityCount() > 0;

        if (!isPlaying) {
            if (!canPlay) ImGui::BeginDisabled();
            if (ImGui::Button("Play") && canPlay) ctx.playSession->startPlaying();
            if (!canPlay) ImGui::EndDisabled();
            if (!canPlay && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(ctx.game.getEntityManager().getEntityCount() == 0
                    ? "Load or create entities first" : "Open viewport first");
            }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.50f, 0.20f, 0.20f, 1.f));
            if (ImGui::Button("Stop Playing")) ctx.playSession->stopPlaying();
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        bool canStop = ctx.state == Engine::GameState::PLAYING || ctx.state == Engine::GameState::PAUSED;
        if (!canStop) ImGui::BeginDisabled();
        if (ImGui::Button("Stop") && canStop) ctx.playSession->stopPlaying();
        if (!canStop) ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Stop and restore entity positions");

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        const char* stateTxt = "EDITING";
        ImVec4 stateCol{0.50f, 0.55f, 0.80f, 1.f};
        if (ctx.state == Engine::GameState::PLAYING) { stateTxt = "PLAYING"; stateCol = {0.30f, 0.78f, 0.70f, 1.f}; }
        else if (ctx.state == Engine::GameState::PAUSED) { stateTxt = "PAUSED"; stateCol = {0.90f, 0.80f, 0.25f, 1.f}; }
        ImGui::TextColored(stateCol, "%s", stateTxt);

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        bool editing = !isPlaying;

        if (!editing) ImGui::BeginDisabled();
        if (ImGui::Button("+ Entity") && editing) ctx.entityOps->addEntity();
        if (!editing) ImGui::EndDisabled();

        ImGui::SameLine();

        bool hasSelected = ctx.selection->current() != nullptr && editing;
        if (!hasSelected) ImGui::BeginDisabled();
        if (ImGui::Button("Duplicate") && hasSelected) ctx.entityOps->duplicateEntity();
        if (!hasSelected) ImGui::EndDisabled();

        ImGui::SameLine();

        bool canDelete = !ctx.selection->all().empty() && editing;
        if (!canDelete) ImGui::BeginDisabled();
        if (ImGui::Button("Delete") && canDelete) ctx.entityOps->deleteSelected();
        if (!canDelete) ImGui::EndDisabled();

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        bool canUndo = ctx.history.canUndo() && ctx.state == Engine::GameState::MENU;
        bool canRedo = ctx.history.canRedo() && ctx.state == Engine::GameState::MENU;
        if (!canUndo) ImGui::BeginDisabled();
        if (ImGui::Button("Undo") && canUndo) {
            std::string desc = ctx.history.undoDescription();
            ctx.history.undo();
            ctx.selection->validate();
            ctx.selection->findPlayer();
            ctx.setStatus("Undo: " + desc);
        }
        if (!canUndo) ImGui::EndDisabled();
        if (canUndo && ImGui::IsItemHovered())
            ImGui::SetTooltip("Undo: %s", ctx.history.undoDescription().c_str());

        ImGui::SameLine();
        if (!canRedo) ImGui::BeginDisabled();
        if (ImGui::Button("Redo") && canRedo) {
            ctx.history.redo();
            ctx.selection->validate();
            ctx.selection->findPlayer();
            ctx.setStatus("Redo: " + ctx.history.undoDescription());
        }
        if (!canRedo) ImGui::EndDisabled();
        if (canRedo && ImGui::IsItemHovered())
            ImGui::SetTooltip("Redo: %s", ctx.history.redoDescription().c_str());

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        if (isPlaying) ImGui::BeginDisabled();
        if (ImGui::Button("Levels")) {
            ctx.levelIO->scanLevelFiles();
            ctx.popups->openLevelBrowser();
        }
        if (isPlaying) ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Browse levels in assets/levels/");

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        bool gridOn = ctx.viewport.gridEnabled();
        if (gridOn) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.42f, 0.38f, 1.f));
        if (ImGui::Button("Grid")) ctx.viewport.gridEnabled() = !ctx.viewport.gridEnabled();
        if (gridOn) ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) {
            if (gridOn)
                ImGui::SetTooltip("Grid: ON (%.0fpx) [G] — click to disable", ctx.viewport.gridSize());
            else
                ImGui::SetTooltip("Grid: OFF [G] — click to enable snap");
        }

        ImGui::SameLine();
        ImGui::Text("| Entities: %zu", ctx.game.getEntityManager().getEntityCount());
        if (ctx.selection->all().size() > 1) {
            ImGui::SameLine();
            ImGui::TextDisabled("(%zu sel)", ctx.selection->all().size());
        }

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();
        int zoomPct = (int)(ctx.viewport.getZoom() * 100.f + 0.5f);
        ImGui::SetNextItemWidth(60);
        if (ImGui::DragInt("##Zoom", &zoomPct, 1, 25, 400, "%d%%")) {
            float newZoom = std::clamp(zoomPct / 100.f, 0.25f, 4.f);
            ctx.viewport.setZoom(newZoom);
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Viewport zoom (scroll in viewport)");

        if (!isPlaying) {
            ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();
            ImGui::TextDisabled("Lvl:"); ImGui::SameLine();
            int lw = (int)ctx.levelIO->levelWidth();
            ImGui::SetNextItemWidth(65);
            if (ImGui::DragInt("##lvlw", &lw, 4, 200, 10000, "%dw")) {
                ctx.levelIO->setLevelSize((float)std::max(200, lw), ctx.levelIO->levelHeight());
                ctx.viewport.setLevelSize(ctx.levelIO->levelWidth(), ctx.levelIO->levelHeight());
                ctx.markDirty();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Level width (px) — drag to resize");
            ImGui::SameLine();
            int lh = (int)ctx.levelIO->levelHeight();
            ImGui::SetNextItemWidth(65);
            if (ImGui::DragInt("##lvlh", &lh, 4, 200, 10000, "%dh")) {
                ctx.levelIO->setLevelSize(ctx.levelIO->levelWidth(), (float)std::max(200, lh));
                ctx.viewport.setLevelSize(ctx.levelIO->levelWidth(), ctx.levelIO->levelHeight());
                ctx.markDirty();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Level height (px) — drag to resize");
            ImGui::SameLine();
            if (ImGui::Button("Fit")) ctx.levelIO->fitLevelToContent();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Auto-resize level bounds to fit all entities (+ padding)");
        }

        float toolbarW = ImGui::GetWindowWidth();
        if (toolbarW < 700) { ImGui::End(); ImGui::PopStyleVar(); return; }

        ImGui::SameLine(toolbarW - 320);
        ImGui::Text("|");
        ImGui::SameLine();

        auto activeCol = ImVec4(0.18f, 0.32f, 0.36f, 1.f);
        auto hiddenCol = ImVec4(0.30f, 0.16f, 0.16f, 1.f);

        ImGui::PushStyleColor(ImGuiCol_Button, ctx.scenePanel.isVisible() ? activeCol : hiddenCol);
        if (ImGui::Button("Scene")) ctx.scenePanel.isVisible() = !ctx.scenePanel.isVisible();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ctx.propsPanel.isVisible() ? activeCol : hiddenCol);
        if (ImGui::Button("Properties")) ctx.propsPanel.isVisible() = !ctx.propsPanel.isVisible();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ctx.viewport.isVisible() ? activeCol : hiddenCol);
        if (ImGui::Button("Viewport")) ctx.viewport.isVisible() = !ctx.viewport.isVisible();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ctx.palette.isVisible() ? activeCol : hiddenCol);
        if (ImGui::Button("Palette")) ctx.palette.isVisible() = !ctx.palette.isVisible();
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

}
