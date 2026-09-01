#pragma once

#include <compare>

#include "others/setting.hpp"
#include "others/util.hpp"

namespace abm {
class Date final {
  public:
    explicit constexpr Date(const int year, const int month, const int day) noexcept
        : year_{year}, month_{month}, day_{day} {
        ASSERT(year > 0);
        ASSERT(month > 0);
        ASSERT(day > 0);
    }

    [[nodiscard]] constexpr auto operator<=>(const Date&) const noexcept -> auto = default;
    [[nodiscard]] constexpr auto operator==(const Date&) const noexcept -> bool  = default;

    [[nodiscard]] constexpr auto isBeginingMonth() const noexcept -> bool { return day_ == 1U; }
    [[nodiscard]] constexpr auto isBeginingYear() const noexcept -> bool { return year_ == 1U; }
    [[nodiscard]] constexpr auto isEndingMonth() const noexcept -> bool {
        return day_ == setting::dayInMonth;
    }
    [[nodiscard]] constexpr auto isEndingYear() const noexcept -> bool {
        return month_ == setting::monthInYear;
    }

    [[nodiscard]] constexpr auto toFlatTime() const noexcept -> int {
        const auto yearIdx   = year_ - 1;
        const auto monthIdx  = month_ - 1;
        const auto dayIdx    = day_ - 1;
        const auto flatMonth = (yearIdx * setting::monthInYear) + monthIdx;
        const auto flatDay   = (flatMonth * setting::dayInMonth) + dayIdx;
        return flatDay;
    }

    [[nodiscard]] constexpr auto isSameDay(const int day) const noexcept -> bool {
        return day == day_;
    }

    constexpr auto operator++() noexcept -> Date& {
        if (day_ < setting::dayInMonth) {
            ++day_;
            return *this;
        }
        if (month_ < setting::monthInYear) {
            day_ = 1U;
            ++month_;
            return *this;
        }
        day_   = 1U;
        month_ = 1U;
        ++year_;
        return *this;
    }

  private:
    int year_;
    int month_;
    int day_;
};
}  // namespace abm