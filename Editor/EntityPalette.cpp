#include "EntityPalette.h"
#include <imgui.h>
#include <algorithm>
#include <cstring>
#include <filesystem>

namespace Editor {

EntityTemplate EntityPalette::consumeCreateRequest() {
    auto r = m_createRequest;
    m_createRequest = EntityTemplate::None;
    return r;
}

std::string EntityPalette::consumePrefabInstantiateRequest() {
    std::string r = std::move(m_prefabInstantiateRequest);
    m_prefabInstantiateRequest.clear();
    return r;
}

bool EntityPalette::consumePrefabSaveRequest(std::string& outName) {
    if (!m_prefabSaveRequested) return false;
    outName = m_prefabSaveName;
    m_prefabSaveName.clear();
    m_prefabSaveRequested = false;
    return true;
}

void EntityPalette::scanPrefabs() {
    m_prefabFiles.clear();
    std::filesystem::path dir = "assets/prefabs";
    std::error_code ec;
    if (!std::filesystem::exists(dir, ec)) return;
    for (auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            m_prefabFiles.push_back(entry.path().string());
    }
    std::sort(m_prefabFiles.begin(), m_prefabFiles.end());
}

void EntityPalette::render() {
    if (!m_visible) return;

    ImGui::SetNextWindowSize(ImVec2(200, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Entity Palette", &m_visible)) { ImGui::End(); return; }

    ImGui::TextDisabled("Click to add to scene");
    ImGui::Separator();
    ImGui::Spacing();

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

        float rowH = 32.f;
        float previewW = std::min(tmpl.w * 0.4f, availW * 0.25f);
        previewW = std::max(previewW, 16.f);
        float previewH = std::min(tmpl.h * 0.4f, rowH - 4.f);
        previewH = std::max(previewH, 8.f);

        ImVec2 cursor = ImGui::GetCursorScreenPos();
        if (ImGui::Selectable("##sel", false, 0, ImVec2(availW, rowH))) {
            m_createRequest = tmpl.id;
        }

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

    ImGui::Spacing();
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Prefabs", ImGuiTreeNodeFlags_DefaultOpen)) {
        static char saveBuf[64] = {0};
        ImGui::TextDisabled("Save selection as");
        ImGui::SetNextItemWidth(-60);
        ImGui::InputText("##prefabname", saveBuf, sizeof(saveBuf));
        ImGui::SameLine();
        if (ImGui::Button("Save") && saveBuf[0]) {
            m_prefabSaveName = saveBuf;
            m_prefabSaveRequested = true;
            saveBuf[0] = 0;
        }

        ImGui::Spacing();
        if (ImGui::Button("Rescan", ImVec2(-1, 0))) scanPrefabs();

        ImGui::Spacing();
        if (m_prefabFiles.empty()) {
            ImGui::TextDisabled("(no prefabs in assets/prefabs)");
        } else {
            for (const auto& path : m_prefabFiles) {
                auto slash = path.find_last_of('/');
                std::string label = (slash != std::string::npos) ? path.substr(slash + 1) : path;
                if (label.size() > 5 && label.substr(label.size() - 5) == ".json")
                    label = label.substr(0, label.size() - 5);
                if (ImGui::Selectable(label.c_str(), false))
                    m_prefabInstantiateRequest = path;
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Click to instantiate at viewport center");
            }
        }
    }

    ImGui::End();
}

}
