#pragma once

#include <compare>

#include "others/util.hpp"

namespace abm {
class Date final {
  public:
    explicit constexpr Date(
        const unsigned int year, const unsigned int month, const unsigned int day
    ) noexcept
        : year_{year}, month_{month}, day_{day} {
        ASSERT(year > 0U);
        ASSERT(month > 0U);
        ASSERT(day > 0U);
    }

    [[nodiscard]] auto operator<=>(const Date&) const noexcept -> auto = default;
    [[nodiscard]] auto operator==(const Date&) const noexcept -> bool  = default;

    [[nodiscard]] auto isBeginingMonth() const noexcept -> bool { return day_ == 1U; }
    [[nodiscard]] auto isBeginingYear() const noexcept -> bool { return year_ == 1U; }

    [[nodiscard]] auto toFlatTime() const noexcept -> unsigned int {
        const auto yearIdx   = year_ - 1U;
        const auto monthIdx  = month_ - 1U;
        const auto dayIdx    = day_ - 1U;
        const auto flatMonth = (yearIdx * 12U) + monthIdx;
        const auto flatDay   = (flatMonth * 30U) + dayIdx;
        return flatDay;
    }

    auto operator++() noexcept -> Date& {
        if (day_ < 30U) {
            ++day_;
            return *this;
        }
        if (month_ < 12U) {
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
    unsigned int year_;
    unsigned int month_;
    unsigned int day_;
};
}  // namespace abm