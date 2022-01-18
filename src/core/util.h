#pragma once

#include <cstdint>
#include <string>
#include <ctime>

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

    class Date {
    private:
        uint8_t m_day = 0;
        uint8_t m_month = 0;
        uint16_t m_year = 0;

        bool m_is_valid = false;

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

        std::string format(std::string fmt);
    };

    Date getCurrentDate();
}