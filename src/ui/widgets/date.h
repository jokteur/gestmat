#pragma once

#include "ui/drawable.h"
#include "core/util.h"

using namespace core;

int FilterInput(ImGuiInputTextCallbackData* data);

class DateWidget : public Drawable {
private:
    enum Focus { DAY, MONTH, YEAR, NONE };

    std::string m_day = "";
    std::string m_month = "";
    std::string m_year = "";

    Focus m_focus = DAY;
    Focus m_current_focus;

    bool m_change_focus = false;

    bool m_day_error = false;
    bool m_month_error = false;
    bool m_year_error = false;

    long long int m_id;
    void input(Focus which);
    void setFocus();

    friend int FilterInput(ImGuiInputTextCallbackData* data);
public:
    DateWidget(UIState_ptr ui_state);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;

    void setToday();
    void setDate(uint8_t day, uint8_t month, uint16_t year);

    Date getDate();

    void reset();
};