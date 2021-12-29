#include "util.h"


core::Date::Date(uint8_t day, uint8_t month, uint16_t year) {
    m_day = day;
    m_month = month;
    m_year = year;

    if (!verifyDate(day, month, year)) {
        m_day = 0;
        m_month = 0;
        m_year = 0;
    }
}

std::string core::Date::format(std::string fmt) {
    if (m_day == 0 || m_month == 0 || m_year == 0)
        return "";

    std::string return_str;

    bool modifier_flag = false;
    for (auto c : fmt) {
        if (modifier_flag) {
            switch (c) {
            case 'd':
                if (m_day < 10)
                    return_str += '0';
                return_str += std::to_string(m_day);
                break;
            case 'm':
                if (m_month < 10)
                    return_str += '0';
                return_str += std::to_string(m_month);
                break;
            case 'Y':
                return_str += std::to_string(m_year);
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

    return core::Date(now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
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