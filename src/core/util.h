#pragma once

#include <cstdint>
#include <string>
#include <ctime>

#include <nlohmann/json.hpp>

namespace Debug {
    class Debugger;
}

namespace core {
    /**
     * @brief Verifies if a date is correct
     *
     * @param day
     * @param month
     * @param year
     *
     * Returns true if the date is valid
     */
    bool verifyDate(uint8_t day, uint8_t month, uint16_t year);

    long long int getTimestamp();

    class Date {
    private:
        uint8_t day = 0;
        uint8_t month = 0;
        uint16_t year = 0;

        bool is_valid = false;

    public:
        /**
         * @brief Construct an empty Date object
         */
        Date() = default;

        /**
         * @brief Construct a new Date object
         * If the date is not a valid date, then it stores day=0, month=0 and year=0
         * @param day
         * @param month
         * @param year
         */
        Date(uint8_t day, uint8_t month, uint16_t year);

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Date, day, month, year, is_valid);

        friend int getDifference(Date date_start, Date date_end);

        uint8_t getDay() { return day; }
        uint8_t getMonth() { return month; }
        uint16_t getYear() { return year; }

        bool isValid() { return is_valid; }
        std::string format(std::string fmt);
    };

    long long int toTimestamp(Date date);

    Date getCurrentDate();

    enum Duration {
        ONE_WEEK,
        TWO_WEEKS,
        THREE_WEEKS,
        ONE_MONTH,
        TWO_MONTHS,
        THREE_MONTHS,
        FOUR_MONTHS,
        FIVE_MONTHS,
        SIX_MONTHS,
        SEVEN_MONTHS,
        HEIGHT_MONTHS,
        NINE_MONTHS,
        TEN_MONTHS,
        ELEVEN_MONTHS,
        ONE_YEAR
    };


    int day_of_year(int y, int m, int d);

    int day_of_week(int y, int m, int d);

    std::map<Duration, std::string> getDurations();

    int getDifference(Date date_start, Date date_end);

    bool isLessThanDuration(Duration duration, Date date_start, Date date_end = Date());

    std::string durationToText(Duration duration);

    std::string toLower(const std::string& str);

    template<typename T>
    class CheckBoxMap {
    private:
        std::map<T, bool> map;
    public:
        CheckBoxMap() {}

        bool& operator[](T prop_id) {
            if (!map.contains(prop_id)) {
                map[prop_id] = false;
            }
            return map[prop_id];
        }

        void erase(T it) {
            map.erase(it);
        }

        std::map<T, bool>::iterator begin() {
            return map.begin();
        }
        std::map<T, bool>::iterator end() {
            return map.end();
        }

        std::map<T, bool> get() {
            return map;
        }

        void clear() {
            map.clear();
        }
    };
}