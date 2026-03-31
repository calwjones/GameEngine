#include "OverlayRenderer.h"
#include "EditorContext.h"
#include "SelectionController.h"
#include "LevelIO.h"
#include "PlaySessionController.h"
#include <imgui.h>
#include <cstdio>

namespace Editor {

void OverlayRenderer::render() {
    if (!ctx.viewport.isVisible()) return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    float cx = ctx.window.getSize().x / 2.f, cy = ctx.window.getSize().y / 2.f;

    if (ctx.state == Engine::GameState::MENU && ctx.game.getEntityManager().getEntityCount() > 0) {
        ctx.selection->findPlayer();

        ImGui::SetNextWindowPos(ImVec2(cx - 150, cy - 50));
        ImGui::SetNextWindowBgAlpha(0.85f);
        if (ImGui::Begin("##Menu", nullptr, flags)) {
            ImGui::Dummy(ImVec2(300, 10));
            ImGui::SetCursorPosX((300 - ImGui::CalcTextSize("2D GAME ENGINE").x) / 2);
            ImGui::TextColored(ImVec4(0.30f, 0.78f, 0.70f, 1.f), "2D GAME ENGINE");
            ImGui::Spacing(); ImGui::Spacing();
            if (ctx.selection->player()) {
                ImGui::SetCursorPosX((300 - ImGui::CalcTextSize("Press SPACE to play").x) / 2);
                ImGui::TextDisabled("Press SPACE to play");
            } else {
                const char* hint = "Add a Player entity to play";
                ImGui::SetCursorPosX((300 - ImGui::CalcTextSize(hint).x) / 2);
                ImGui::TextColored(ImVec4(0.90f, 0.65f, 0.20f, 0.8f), "%s", hint);
            }
            ImGui::Dummy(ImVec2(300, 10));
        }
        ImGui::End();
    }

    if (ctx.state == Engine::GameState::PAUSED) {
        ImGui::SetNextWindowPos(ImVec2(cx - 120, cy - 60));
        ImGui::SetNextWindowBgAlpha(0.85f);
        if (ImGui::Begin("##Pause", nullptr, flags)) {
            ImGui::Dummy(ImVec2(240, 10));
            ImGui::SetCursorPosX((240 - ImGui::CalcTextSize("PAUSED").x) / 2);
            ImGui::TextColored(ImVec4(1.f, 0.8f, 0.2f, 1.f), "PAUSED");
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            ImGui::TextDisabled("ESC  - Resume");
            ImGui::TextDisabled("Q    - Stop & Edit");
            ImGui::Dummy(ImVec2(240, 10));
        }
        ImGui::End();
    }

    PlayOverlay overlay = ctx.playSession->overlay();
    if (overlay != PlayOverlay::None &&
        (ctx.state == Engine::GameState::PLAYING || ctx.state == Engine::GameState::PAUSED)) {

        ImGui::SetNextWindowPos(ImVec2(cx - 180, cy - 110));
        ImGui::SetNextWindowBgAlpha(0.92f);
        if (ImGui::Begin("##Overlay", nullptr, flags)) {
            constexpr float PANEL_W = 360.f;
            ImGui::Dummy(ImVec2(PANEL_W, 8));

            const char* title = "";
            ImVec4 titleCol{1.f, 1.f, 1.f, 1.f};
            const char* btnLabel = "Continue";
            switch (overlay) {
                case PlayOverlay::LevelComplete:
                    title = "LEVEL COMPLETE"; titleCol = {0.55f, 0.90f, 1.0f, 1.f};
                    btnLabel = "Next Level"; break;
                case PlayOverlay::WorldComplete:
                    title = "WORLD COMPLETE!"; titleCol = {0.30f, 0.98f, 0.60f, 1.f};
                    btnLabel = "Back to Editor"; break;
                case PlayOverlay::GameOver:
                    title = "GAME OVER"; titleCol = {1.00f, 0.35f, 0.35f, 1.f};
                    btnLabel = "Retry World"; break;
                case PlayOverlay::SingleWin:
                    title = "YOU WIN!"; titleCol = {0.30f, 0.98f, 0.60f, 1.f};
                    btnLabel = "Back to Editor"; break;
                case PlayOverlay::None: break;
            }
            ImGui::SetCursorPosX((PANEL_W - ImGui::CalcTextSize(title).x) / 2);
            ImGui::TextColored(titleCol, "%s", title);

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            float playTime = ctx.playSession->gameplay().playTime();
            int mins = (int)(playTime / 60.f);
            float secs = playTime - mins * 60;
            char timeBuf[32];
            if (mins > 0) snprintf(timeBuf, sizeof(timeBuf), "%dm %.1fs", mins, secs);
            else          snprintf(timeBuf, sizeof(timeBuf), "%.1fs", secs);

            auto centerLine = [&](const char* text, ImVec4 col) {
                ImGui::SetCursorPosX((PANEL_W - ImGui::CalcTextSize(text).x) / 2);
                ImGui::TextColored(col, "%s", text);
            };

            char buf[80];
            const auto& ogs = ctx.playSession->groupSession();
            if (ogs.active &&
                ogs.groupIndex >= 0 &&
                ogs.groupIndex < (int)ctx.levelIO->groups().size()) {
                auto& g = ctx.levelIO->groups()[ogs.groupIndex];
                snprintf(buf, sizeof(buf), "%s", g.name.c_str());
                centerLine(buf, ImVec4(0.75f, 0.85f, 1.f, 1.f));

                if (overlay == PlayOverlay::LevelComplete ||
                    overlay == PlayOverlay::WorldComplete) {
                    snprintf(buf, sizeof(buf), "Level %d / %d cleared",
                             ogs.currentLevel + 1, (int)g.levels.size());
                    centerLine(buf, ImVec4(0.85f, 0.85f, 0.85f, 1.f));
                } else if (overlay == PlayOverlay::GameOver) {
                    snprintf(buf, sizeof(buf), "Died on Level %d / %d",
                             ogs.currentLevel + 1, (int)g.levels.size());
                    centerLine(buf, ImVec4(0.9f, 0.5f, 0.5f, 1.f));
                }

                ImGui::Spacing();

                snprintf(buf, sizeof(buf), "Level Coins: %d", ctx.playSession->levelCoinsSnapshot());
                centerLine(buf, ImVec4(1.f, 0.84f, 0.f, 1.f));

                snprintf(buf, sizeof(buf), "World Total: %d", ogs.totalCoins);
                centerLine(buf, ImVec4(1.f, 0.70f, 0.20f, 1.f));

                snprintf(buf, sizeof(buf), "Lives Remaining: %d", ogs.livesRemaining);
                centerLine(buf, ImVec4(1.f, 0.45f, 0.45f, 1.f));
            } else {
                snprintf(buf, sizeof(buf), "Score: %d", ctx.playSession->score());
                centerLine(buf, ImVec4(1.f, 0.84f, 0.f, 1.f));
                snprintf(buf, sizeof(buf), "Time:  %s", timeBuf);
                centerLine(buf, ImVec4(0.7f, 0.7f, 0.7f, 1.f));
            }

            ImGui::Spacing(); ImGui::Spacing();
            ImGui::SetCursorPosX((PANEL_W - 180.f) / 2.f);
            if (ImGui::Button(btnLabel, ImVec2(180, 0))) {
                switch (overlay) {
                    case PlayOverlay::LevelComplete:
                        ctx.playSession->setOverlay(PlayOverlay::None);
                        ctx.playSession->advanceGroupLevel();
                        break;
                    case PlayOverlay::GameOver:
                        ctx.playSession->setOverlay(PlayOverlay::None);
                        ctx.playSession->restartGroup();
                        break;
                    case PlayOverlay::WorldComplete:
                    case PlayOverlay::SingleWin:
                        ctx.playSession->setOverlay(PlayOverlay::None);
                        ctx.playSession->stopPlaying();
                        break;
                    case PlayOverlay::None: break;
                }
            }

            ImGui::Dummy(ImVec2(PANEL_W, 8));
        }
        ImGui::End();
    }
}

}
