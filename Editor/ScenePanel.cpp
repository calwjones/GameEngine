#include "ScenePanel.h"
#include "../Engine/Entity/EntityTypeRegistry.h"
#include <imgui.h>
#include <algorithm>
#include <cctype>

namespace Editor {

Engine::Entity* ScenePanel::consumeDeleteRequest() {
    auto* e = m_deleteRequest;
    m_deleteRequest = nullptr;
    return e;
}

Engine::Entity* ScenePanel::consumeDuplicateRequest() {
    auto* e = m_duplicateRequest;
    m_duplicateRequest = nullptr;
    return e;
}

bool ScenePanel::consumeAddRequest() {
    bool r = m_addRequest;
    m_addRequest = false;
    return r;
}

Engine::Entity* ScenePanel::consumeMoveUpRequest() {
    auto* e = m_moveUpRequest;
    m_moveUpRequest = nullptr;
    return e;
}

Engine::Entity* ScenePanel::consumeMoveDownRequest() {
    auto* e = m_moveDownRequest;
    m_moveDownRequest = nullptr;
    return e;
}

void ScenePanel::render(Engine::EntityManager& mgr,
                        std::vector<Engine::Entity*>& selection,
                        Engine::Entity*& primary,
                        bool playing) {
    if (!m_visible) return;

    ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Scene", &m_visible)) { ImGui::End(); return; }

    ImGui::Text("Entities");
    ImGui::Separator();

    char buf[128] = {0};
    snprintf(buf, sizeof(buf), "%s", m_filter.c_str());
    float clearBtnW = m_filter.empty() ? 0.f : 22.f;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - clearBtnW);
    if (ImGui::InputTextWithHint("##Search", "Search...", buf, sizeof(buf)))
        m_filter = buf;
    if (!m_filter.empty()) {
        ImGui::SameLine(0, 2);
        if (ImGui::SmallButton("X")) m_filter.clear();
    }

    ImGui::Separator();

    auto isSelected = [&](Engine::Entity* e) {
        return std::find(selection.begin(), selection.end(), e) != selection.end();
    };

    auto& ents = mgr.getAllEntities();
    if (ents.empty()) {
        ImGui::Spacing();
        ImGui::TextDisabled("No entities in scene");
        ImGui::Spacing();
        if (ImGui::Button("+ Add Entity", ImVec2(-1, 0)))
            m_addRequest = true;
        ImGui::Spacing();
        ImGui::TextDisabled("Or use the Entity Palette");

        if (ImGui::BeginPopupContextWindow("##EmptyContext")) {
            if (ImGui::MenuItem("Add Entity")) m_addRequest = true;
            ImGui::EndPopup();
        }
    } else {
        bool childVisible = ImGui::BeginChild("List", ImVec2(0, -25), true);
        if (childVisible) {
            auto toLower = [](std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; };
            std::string filterLow = toLower(m_filter);

            for (size_t idx = 0; idx < ents.size(); idx++) {
                auto* e = ents[idx];
                if (!m_filter.empty() && toLower(e->name).find(filterLow) == std::string::npos) continue;

                ImGui::PushID(e);

                const char* icon = "[-]";
                ImVec4 col{1, 1, 1, 1};
                if (const auto* info = Engine::findEntityType(e->type)) {
                    icon = info->icon;
                    col = {info->r, info->g, info->b, 1.f};
                }

                ImGui::TextColored(col, "%s", icon);
                ImGui::SameLine();

                bool selected = isSelected(e);
                if (ImGui::Selectable(e->name.c_str(), selected)) {
                    bool ctrl = ImGui::GetIO().KeyCtrl;
                    if (ctrl) {
                        if (selected) {
                            selection.erase(std::remove(selection.begin(), selection.end(), e), selection.end());
                            if (primary == e)
                                primary = selection.empty() ? nullptr : selection.back();
                        } else {
                            selection.push_back(e);
                            primary = e;
                        }
                    } else {
                        selection.clear();
                        selection.push_back(e);
                        primary = e;
                    }
                }

                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Duplicate", nullptr, false, !playing)) m_duplicateRequest = e;
                    ImGui::Separator();
                    if (ImGui::MenuItem("Move Up", nullptr, false, !playing && idx + 1 < ents.size())) m_moveUpRequest = e;
                    if (ImGui::MenuItem("Move Down", nullptr, false, !playing && idx > 0)) m_moveDownRequest = e;
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete", nullptr, false, !playing)) m_deleteRequest = e;
                    ImGui::EndPopup();
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Type: %s", e->type.c_str());
                    ImGui::Text("Pos: (%.1f, %.1f)", e->position.x, e->position.y);
                    ImGui::Text("Size: %.0fx%.0f", e->size.x, e->size.y);
                    auto eBounds = e->getBounds();
                    for (size_t k = 0; k < ents.size(); k++) {
                        if (k == idx) continue;
                        if (eBounds.intersects(ents[k]->getBounds())) {
                            ImGui::TextColored(ImVec4(1.f, 0.6f, 0.2f, 1.f), "Overlaps: %s", ents[k]->name.c_str());
                            break;
                        }
                    }
                    ImGui::EndTooltip();
                }
                ImGui::PopID();
            }

            if (ImGui::BeginPopupContextWindow("##ListContext", ImGuiPopupFlags_NoOpenOverItems)) {
                if (ImGui::MenuItem("Add Entity")) m_addRequest = true;
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
    }

    ImGui::Separator();
    ImGui::Text("Total: %zu", ents.size());
    if (selection.size() > 1) {
        ImGui::SameLine();
        ImGui::TextDisabled("(%zu selected)", selection.size());
    }
    ImGui::End();
}

}
