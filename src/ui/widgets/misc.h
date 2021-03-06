#pragma once

#include <string>
#include <chrono>
#include <tempo.h>

#include "ui/state.h"

#include "imgui_internal.h"
#include "imgui_stdlib.h"

struct DateTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};

DateTime getDatetime(long long int timestamp);

std::string format_CET(long long int timestamp);

void title(std::string title, UIState_ptr ui_state);

void boldAndNormal(std::string bold, std::string normal, UIState_ptr ui_state);

bool button(std::string name, UIState_ptr ui_state, std::string deactivated_msg = "", ImVec4 color = ImVec4(), bool ignore_deactivated = false, ImVec2 size = ImVec2());

void labeledTextInput(std::string* content, const std::string& label, const std::string& imId, const std::string& hint, const std::string& error = "", ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = (ImGuiInputTextCallback)0, void* user_data = (void*)0);

void help(std::string content);

void timestampToText(long long int timestamp);