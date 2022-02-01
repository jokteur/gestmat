#pragma once

#include "ui/drawable.h"

typedef std::set<std::string> Filter;

class FilterWidget : public Drawable {
private:
    std::string m_filter;
    std::string m_sanitized_input;
    long long m_id;
    std::string m_hint;
public:
    FilterWidget(UIState_ptr ui_state, std::string hint);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;

    bool PassFilter(const std::set<std::string>& input);
};