#pragma once

#include <cstdint>
#include <string>
#include <ctime>

#include <nlohmann/json.hpp>

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

        uint8_t getDay() { return day; }
        uint8_t getMonth() { return month; }
        uint16_t getYear() { return year; }

        bool isValid() { return is_valid; }
        std::string format(std::string fmt);
    };

    Date getCurrentDate();

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