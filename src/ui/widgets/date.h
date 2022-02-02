#pragma once

#include "ui/drawable.h"
#include "core/util.h"

using namespace core;

class DateWidget : public Drawable {
private:
    enum Focus { DAY, MONTH, YEAR };
    std::string m_day;
    std::string m_month;
    std::string m_year;
    Focus m_focus = DAY;

    long long int m_id;

    void setFocus();
public:
    DateWidget(UIState_ptr ui_state);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override {}

    Date getDate();

    void reset();
};