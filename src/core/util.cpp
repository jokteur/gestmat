#include "util.h"

#include <chrono>
#include <ctime>
#include "python/py_api.h"

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

long long int core::toTimestamp(core::Date date) {
    int y = (int)date.getYear();
    int m = (int)date.getMonth();
    int d = (int)date.getDay();


    std::tm tm;
    tm.tm_hour = 12;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_year = y - 1900;
    tm.tm_mon = m - 1;
    tm.tm_mday = d;
    // tm.tm_wday = day_of_week(y, m, d);
    // tm.tm_yday = day_of_year(y, m, d);

    time_t time = std::mktime(&tm);

    return time;
}

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

int daysToMonth[2][12] =
{
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 },
};


bool isLeapYear(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int core::day_of_year(int y, int m, int d) {
    return daysToMonth[isLeapYear(y) ? 1 : 0][m] + d;
}

int core::day_of_week(int y, int m, int d) {
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    y -= m < 3;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
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

std::string core::durationToText(Duration duration) {
    switch (duration) {
    case ONE_WEEK:
        return "Une semaine";
    case TWO_WEEKS:
        return "Deux semaines";
    case THREE_WEEKS:
        return "Trois semaines";
    case ONE_MONTH:
        return "Un mois";
    case TWO_MONTHS:
        return "Deux mois";
    case THREE_MONTHS:
        return "Trois mois";
    case FOUR_MONTHS:
        return "Quatre mois";
    case FIVE_MONTHS:
        return "Cinq mois";
    case SIX_MONTHS:
        return "Six mois";
    case SEVEN_MONTHS:
        return "Sept mois";
    case HEIGHT_MONTHS:
        return "Huit mois";
    case NINE_MONTHS:
        return "Neuf mois";
    case TEN_MONTHS:
        return "Dix mois";
    case ELEVEN_MONTHS:
        return "Onze mois";
    case ONE_YEAR:
        return "Une année";
    };
    return "inconnu";
}

std::map<core::Duration, std::string> core::getDurations() {
    std::map<core::Duration, std::string> map;
    for (int i = 0;i < 15;i++) {
        map[(Duration)i] = durationToText((Duration)i);
    }
    return map;
}

int countLeapYears(core::Date d) {
    int years = d.getYear();

    // Check if the current year needs to be
    //  considered for the count of leap years
    // or not
    if (d.getMonth() <= 2)
        years--;

    // An year is a leap year if it
    // is a multiple of 4,
    // multiple of 400 and not a
     // multiple of 100.
    return years / 4
        - years / 100
        + years / 400;
}

int core::getDifference(Date date_start, Date date_end) {
    if (!date_start.isValid() && !date_end.isValid())
        return 0;

    long int n1 = date_start.year * 365 + date_start.day + daysToMonth[0][date_start.month - 1];
    long int n2 = date_end.year * 365 + date_end.day + daysToMonth[0][date_end.month - 1];

    n1 += countLeapYears(date_start);
    n2 += countLeapYears(date_end);

    int diff = n2 - n1;
    return diff;
}

bool core::isLessThanDuration(Duration duration, Date date_start, Date date_end) {
    if (!date_end.isValid())
        date_end = getCurrentDate();

    int diff = getDifference(date_start, date_end);
    switch (duration) {
    case ONE_WEEK:
        return diff < 7;
    case TWO_WEEKS:
        return diff < 14;
    case THREE_WEEKS:
        return diff < 21;
    case ONE_MONTH:
        return diff < 30;
    case TWO_MONTHS:
        return diff < 60;
    case THREE_MONTHS:
        return diff < 90;
    case FOUR_MONTHS:
        return diff < 120;
    case FIVE_MONTHS:
        return diff < 150;
    case SIX_MONTHS:
        return diff < 180;
    case SEVEN_MONTHS:
        return diff < 210;
    case HEIGHT_MONTHS:
        return diff < 240;
    case NINE_MONTHS:
        return diff < 270;
    case TEN_MONTHS:
        return diff < 300;
    case ELEVEN_MONTHS:
        return diff < 330;
    case ONE_YEAR:
        return diff < 365;
    };
    return false;
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