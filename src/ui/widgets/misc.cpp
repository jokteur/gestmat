#include "misc.h"

#include <chrono>
#include <string>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>


void title(std::string title, UIState_ptr ui_state) {
    Tempo::PushFont(ui_state->font_title);
    ImGui::Text(title.c_str());
    Tempo::PopFont();
}

bool button(std::string name, UIState_ptr ui_state, std::string deactivated_msg, ImVec4 color, bool ignore_deactivated, ImVec2 size) {
    bool is_deactivated = (ui_state->read_only || !deactivated_msg.empty()) && !ignore_deactivated;
    bool colored = color.w != 0.f && color.x != 0.f && color.y != 0.f && color.z != 0.f;
    if (colored) {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x * 0.9f, color.y * 0.9f, color.z * 0.9f, color.w + 0.1f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x * 0.8f, color.y * 0.8f, color.z * 0.8f, color.w + 0.2f));
    }
    if (is_deactivated) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    bool ret = ImGui::Button(name.c_str(), size);
    if (is_deactivated) {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::BeginTooltip();
            if (!deactivated_msg.empty())
                ImGui::Text(deactivated_msg.c_str());
            else
                ImGui::Text("Boutton désactivé temporairement");
            ImGui::EndTooltip();
        }
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    if (colored) {
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }
    return ret & !is_deactivated;
}
void labeledTextInput(std::string* content, const std::string& label, const std::string& imId, const std::string& hint, const std::string& error, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
    if (!label.empty()) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text(label.c_str());
        ImGui::SameLine();
    }
    if (!error.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(125, 0, 0, 79));
    }
    ImGui::InputTextWithHint(imId.c_str(), hint.c_str(), content, flags, callback, user_data);
    if (!error.empty()) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(125, 0, 0, 79));
        ImGui::Text(error.c_str());
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }
}

void help(std::string content) {
    ImGui::TextColored(ImVec4(0.2f, 0.2f, 0.2f, 0.4f), "(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(content.c_str());
        ImGui::EndTooltip();
    }
}

DateTime getDatetime(long long int timestamp) {
    std::chrono::seconds time(timestamp);
    std::chrono::time_point<std::chrono::system_clock> tp(time);
    using namespace std::chrono;
    tp += 1h;

    using days = std::chrono::duration<int, std::ratio<86400>>;

    // Get time_points with both millisecond and day precision
    auto tp_ms = time_point_cast<milliseconds>(tp);
    auto tp_d = time_point_cast<days>(tp_ms);

    // Get {y, m, d} from tp_d
    auto z = tp_d.time_since_epoch().count();
    static_assert(std::numeric_limits<unsigned>::digits >= 18,
        "This algorithm has not been ported to a 16 bit unsigned integer");
    static_assert(std::numeric_limits<int>::digits >= 20,
        "This algorithm has not been ported to a 16 bit signed integer");
    z += 719468;
    const int era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);          // [0, 146096]
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;  // [0, 399]
    int y = static_cast<int>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);                // [0, 365]
    const unsigned mp = (5 * doy + 2) / 153;                                   // [0, 11]
    const unsigned d = doy - (153 * mp + 2) / 5 + 1;                             // [1, 31]
    const unsigned m = mp + (mp < 10 ? 3 : -9);                            // [1, 12]
    y += (m <= 2);

    // Get milliseconds since the local midnight
    auto ms = tp_ms - tp_d;

    // Get {h, M, s, ms} from milliseconds since midnight
    auto h = duration_cast<hours>(ms);
    ms -= h;
    auto M = duration_cast<minutes>(ms);
    ms -= M;
    auto s = duration_cast<seconds>(ms);
    ms -= s;

    return DateTime{
        .year = y,
        .month = (int)m,
        .day = (int)d,
        .hour = h.count(),
        .minute = M.count(),
        .second = (int)s.count()
    };
}

std::string format_CET(long long int timestamp) {
    auto datetime = getDatetime(timestamp);
    // Format {y, m, d, h, M, s, ms} as yyyy-MM-dd'T'HH:mm:ss'.'SSS+0100
    std::ostringstream os;
    os.fill('0');
    os << std::setw(4) << datetime.year << '-'
        << std::setw(2) << datetime.month << '-' << std::setw(2)
        << datetime.day;
    os << ' ';
    os << std::setw(2) << datetime.hour << 'h'
        << std::setw(2) << datetime.minute;
    return os.str();
}

void timestampToText(long long int timestamp) {
    // Date library is cursed on some computers that are
    // not up-to-date, roll my own
    std::string text = format_CET(timestamp);
    ImGui::Text(text.c_str());
}