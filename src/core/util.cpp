#include "util.h"

#include <chrono>
#include "python/py_api.h"

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

core::Date::Date(uint8_t day_, uint8_t month_, uint16_t year_) {
    day = day_;
    month = month_;
    year = year_;

    if (!verifyDate(day_, month_, year_)) {
        day = 0;
        month = 0;
        year = 0;
    }
    else {
        is_valid = true;
    }
}

long long int core::getTimestamp() {
    const auto p1 = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
}

std::string core::Date::format(std::string fmt) {
    if (day == 0 || month == 0 || year == 0)
        return "";

    std::string return_str;

    bool modifier_flag = false;
    for (auto c : fmt) {
        if (modifier_flag) {
            switch (c) {
            case 'd':
                if (day < 10)
                    return_str += '0';
                return_str += std::to_string(day);
                break;
            case 'm':
                if (month < 10)
                    return_str += '0';
                return_str += std::to_string(month);
                break;
            case 'Y':
                return_str += std::to_string(year);
                break;

            default:
                return_str += c;
                break;
            }
            modifier_flag = false;
        }
        else {
            if (c == '%')
                modifier_flag = true;
            else
                return_str += c;
        }
    }

    return return_str;
}

core::Date core::getCurrentDate() {
    std::time_t t = std::time(0);   // get time now
    std::tm* now = std::localtime(&t);

    return core::Date((uint8_t)now->tm_mday, (uint8_t)now->tm_mon + 1, (uint16_t)now->tm_year + 1900);
}

bool core::verifyDate(uint8_t day, uint8_t month, uint16_t year) {
    if (day == 0 || month == 0 || year == 0)
        return false;
    if (month > 12 || day > 31)
        return false;
    if (month == 2) {
        if (day > 29)
            return false;
        else if (day == 29) {
            if (year % 4 == 0) {
                if (year % 400 == 0)
                    return false;
            }
            else {
                return false;
            }
        }
    }
    if (day == 31) {
        if (month == 2 || month == 4 || month == 6 || month == 9 || month == 11)
            return false;
    }
    return true;
}

namespace py = pybind11;

std::string core::toLower(const std::string& str) {
    std::string new_str;
    auto state = PyGILState_Ensure();
    try {
        auto script = py::module::import("python.scripts.util");
        new_str = script.attr("lowerWithAccents")(str).cast<std::string>();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        PyGILState_Release(state);
        return str;
    }

    PyGILState_Release(state);
    return new_str;
}