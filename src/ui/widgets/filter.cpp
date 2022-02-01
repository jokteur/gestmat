#include "filter.h"

#include "imgui_stdlib.h"
#include "ui/imgui_util.h"
#include "core/util.h"

#include<algorithm>

FilterWidget::FilterWidget(UIState_ptr ui_state, std::string hint) : Drawable(ui_state) {
    m_id = m_ui_state->imID;
    m_ui_state->imID++;
    m_hint = hint;
}

void FilterWidget::FrameUpdate() {
    ImGui::InputTextWithHint(labelize(m_id, "##search_").c_str(), m_hint.c_str(), &m_filter);
    m_sanitized_input = core::toLower(m_filter);
}
void FilterWidget::BeforeFrameUpdate() {

}

bool FilterWidget::PassFilter(const std::set<std::string>& input) {
    if (m_filter.empty())
        return true;
    for (const std::string& str : input) {
        if (str.empty())
            continue;
        if (str.find(m_sanitized_input) != std::string::npos) {
            return true;
        }
    }
    return false;
}