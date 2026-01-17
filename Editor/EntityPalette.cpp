#include "EntityPalette.h"
#include <imgui.h>
#include <algorithm>

namespace Editor {

// read/reset/return, called once a frame by EditorApplication
EntityTemplate EntityPalette::consumeCreateRequest() {
    auto r = m_createRequest;
    m_createRequest = EntityTemplate::None;   // MUST reset or wed spawn every frame
    return r;
}

// Selectable claims the row for hover/click, then we draw the swatch + two-line label manually bc imgui has no built-in widget for this layout
void EntityPalette::render() {
    if (!m_visible) return;

    ImGui::SetNextWindowSize(ImVec2(200, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Entity Palette", &m_visible)) { ImGui::End(); return; }

    ImGui::TextDisabled("Click to add to scene");
    ImGui::Separator();
    ImGui::Spacing();

    // w/h kept here bc the swatch is sized proportionally to entity dimensions (visual hint that Ground is bigger than Platform etc)
    struct TemplateInfo {
        EntityTemplate id;
        const char* name;
        const char* desc;
        ImVec4 color;
        float w, h;
    };

    TemplateInfo templates[] = {
        {EntityTemplate::Player,        "Player",         "32x48, gravity",     ImVec4(0.20f, 0.80f, 0.20f, 1.f), 32, 48},
        {EntityTemplate::Enemy,         "Enemy",          "32x32, gravity",     ImVec4(0.86f, 0.08f, 0.24f, 1.f), 32, 32},
        {EntityTemplate::Platform,      "Platform",       "64x16, static",      ImVec4(0.39f, 0.39f, 0.47f, 1.f), 64, 16},
        {EntityTemplate::LargePlatform, "Large Platform", "192x16, static",     ImVec4(0.39f, 0.39f, 0.47f, 1.f), 192, 16},
        {EntityTemplate::Ground,        "Ground",         "800x32, static",     ImVec4(0.24f, 0.24f, 0.31f, 1.f), 800, 32},
        {EntityTemplate::Wall,          "Wall",           "16x96, static",      ImVec4(0.31f, 0.31f, 0.39f, 1.f), 16, 96},
        {EntityTemplate::FlyingEnemy,   "Flying Enemy",   "28x28, sine wave",   ImVec4(0.71f, 0.20f, 0.86f, 1.f), 28, 28},
        {EntityTemplate::ShootingEnemy, "Shooting Enemy", "32x32, fires",       ImVec4(0.86f, 0.47f, 0.08f, 1.f), 32, 32},
        {EntityTemplate::Collectible,   "Collectible",    "16x16, trigger",     ImVec4(1.00f, 0.84f, 0.00f, 1.f), 16, 16},
        {EntityTemplate::MovingPlatform,"Moving Platform","128x16, A→B",        ImVec4(0.24f, 0.47f, 0.71f, 1.f), 128, 16},
        {EntityTemplate::Goal,          "Goal",           "32x48, Win",         ImVec4(1.00f, 0.85f, 0.00f, 1.f), 32, 48},
        {EntityTemplate::Hazard,        "Spikes (Hazard)","32x32, Kill",        ImVec4(0.70f, 0.70f, 0.80f, 1.f), 32, 32},
    };

    float availW = ImGui::GetContentRegionAvail().x;

    for (auto& tmpl : templates) {
        ImGui::PushID((int)tmpl.id);

        // swatch size is proportional to entity dimensions, clamped so it doesnt overflow the row
        float rowH = 32.f;
        float previewW = std::min(tmpl.w * 0.4f, availW * 0.25f);
        previewW = std::max(previewW, 16.f);
        float previewH = std::min(tmpl.h * 0.4f, rowH - 4.f);
        previewH = std::max(previewH, 8.f);

        // Selectable claims the full row for hover/click, then we draw the swatch over it
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        if (ImGui::Selectable("##sel", false, 0, ImVec2(availW, rowH))) {
            m_createRequest = tmpl.id;
        }

        // draw the colour swatch manually using the DrawList api
        ImDrawList* draw = ImGui::GetWindowDrawList();
        float swatchX = cursor.x + 4;
        float swatchY = cursor.y + (rowH - previewH) / 2;
        ImU32 col = ImGui::GetColorU32(tmpl.color);
        draw->AddRectFilled(
            ImVec2(swatchX, swatchY),
            ImVec2(swatchX + previewW, swatchY + previewH),
            col, 2.f);
        draw->AddRect(
            ImVec2(swatchX, swatchY),
            ImVec2(swatchX + previewW, swatchY + previewH),
            ImGui::GetColorU32(ImVec4(1, 1, 1, 0.2f)), 2.f);

        float textX = cursor.x + previewW + 12;
        draw->AddText(ImVec2(textX, cursor.y + 4),
                      ImGui::GetColorU32(ImGuiCol_Text), tmpl.name);
        draw->AddText(ImVec2(textX, cursor.y + 18),
                      ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.55f, 1.f)), tmpl.desc);

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Click to place in scene");

        ImGui::PopID();
    }

    ImGui::End();
}

}
