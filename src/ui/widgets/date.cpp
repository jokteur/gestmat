#include "date.h"

#include "ui/imgui_util.h"
#include "misc.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

#include <cctype>

DateWidget::DateWidget(UIState_ptr ui_state) : Drawable(ui_state) {
    m_id = ui_state->imID;
    ui_state->imID++;
    m_day.reserve(2);
    m_month.reserve(2);
    m_year.reserve(4);
}

void DateWidget::reset() {
    m_id = m_ui_state->imID;
    m_ui_state->imID++;
    m_focus = NONE;
    m_day.clear();
    m_month.clear();
    m_year.clear();
    m_day.reserve(2);
    m_month.reserve(2);
    m_year.reserve(4);
}

void DateWidget::setFocus() {
    if (m_change_focus)
        ImGui::SetKeyboardFocusHere();
    m_change_focus = false;
}

void DateWidget::BeforeFrameUpdate() {
    m_day_error = false;
    m_month_error = false;
    if (strlen(m_day.data())) {
        int day = std::stoi(m_day);
        if (day < 1 || day > 31) {
            m_day_error = true;
        }
    }
    if (strlen(m_month.data())) {
        int month = std::stoi(m_month);
        if (month < 1 || month > 12) {
            m_month_error = true;
        }
    }
}

int FilterInput(ImGuiInputTextCallbackData* data) {
    auto widget = static_cast<DateWidget*>(data->UserData);

    if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) {
        if (widget->m_current_focus == DateWidget::DAY) {
            if (data->BufTextLen == 2) {
                widget->m_change_focus = true;
                widget->m_focus = DateWidget::MONTH;
            }
        }
        else if (widget->m_current_focus == DateWidget::MONTH) {
            if (data->BufTextLen == 2) {
                widget->m_change_focus = true;
                widget->m_focus = DateWidget::YEAR;
            }
        }
        else if (widget->m_current_focus == DateWidget::YEAR) {
            int current_year = getCurrentDate().getYear() - 2000;
            if (data->BufTextLen == 2) {
                int written_year = std::stoi(data->Buf);
                if (written_year != 19 && written_year != 20) {
                    int threshold = current_year - written_year + 1;
                    std::string repl;
                    if (threshold >= 0) {
                        repl = std::to_string(written_year + 2000);
                    }
                    else {
                        repl = std::to_string(written_year + 1900);
                    }
                    for (int i = 0;i < 4;i++)
                        data->Buf[i] = repl[i];
                    data->Buf[4] = 0;
                    data->BufTextLen = (int)strlen(data->Buf);
                    data->BufDirty = true;
                    data->CursorPos = 4;
                }
            }
        }
    }

    if (!isdigit(data->EventChar))
        return 1;
    return 0;
}

void DateWidget::input(Focus which) {
    bool error;
    bool end = false;
    float width = 51.f;
    std::string* content;
    size_t max_size = 3;
    std::string label;
    std::string hint;

    if (which == DAY) {
        error = m_day_error;
        content = &m_day;
        label = labelize(m_id, "##day");
        hint = "Jour";
    }
    else if (which == MONTH) {
        error = m_month_error;
        content = &m_month;
        label = labelize(m_id, "##month");
        hint = "Mois";
    }
    else {
        error = m_year_error;
        content = &m_year;
        label = labelize(m_id, "##year");
        hint = "Année";
        width = 80.f;
        max_size = 5;
        end = true;
    }
    if (error) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(125, 0, 0, 79));
    }
    ImGui::SetNextItemWidth(width);
    if (m_focus == which) {
        setFocus();
    }

    ImGui::InputTextWithHint(label.c_str(), hint.c_str(), content->data(), max_size,
        ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_CallbackEdit
        , FilterInput, this
    );

    if (ImGui::IsItemActive() && !m_change_focus) {
        m_current_focus = which;
    }
    if (!end)
        ImGui::SameLine();
    if (error) {
        ImGui::PopStyleColor();
    }
}

Date DateWidget::getDate() {
    int day = 0;
    int month = 0;
    int year = 0;
    if (strlen(m_day.data())) {
        day = std::stoi(m_day);
    }
    if (strlen(m_month.data())) {
        month = std::stoi(m_month);
    }
    if (strlen(m_year.data())) {
        year = std::stoi(m_year);
    }
    Date date((uint8_t)day, (uint8_t)month, (uint16_t)year);
    return date;
}

void DateWidget::FrameUpdate() {
    input(DAY);
    input(MONTH);
    input(YEAR);
}