#pragma once

#include <compare>

#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/mixin.hpp"

namespace abm {
class Day final : public value_object::BaseValueObject<int>, value_object::CompareMixin<Day> {
  public:
    explicit constexpr Day(const int day) noexcept : BaseValueObject<int>(day) { ASSERT(day > 0); }

    [[nodiscard]] constexpr auto operator+(Day other) const noexcept -> Day {
        return Day{value_ + other.value_};
    }
    [[nodiscard]] constexpr auto operator-(Day other) const noexcept -> Day {
        return Day{value_ - other.value_};
    }

    constexpr auto operator++() noexcept -> Day& {
        ++value_;
        return *this;
    }
};

class Month final : public value_object::BaseValueObject<int>, value_object::CompareMixin<Month> {
  public:
    explicit constexpr Month(const int month) noexcept : BaseValueObject<int>(month) {
        ASSERT(month > 0);
    }
    constexpr auto operator++() noexcept -> Month& {
        ++value_;
        return *this;
    }
    constexpr auto operator-(const Month other) const noexcept -> Month {
        return Month{value_ - other.value_};
    }
};

class Year final : public value_object::BaseValueObject<int>, value_object::CompareMixin<Year> {
  public:
    explicit constexpr Year(const int year) noexcept : BaseValueObject<int>(year) {
        ASSERT(year > 0);
    }

    [[nodiscard]] constexpr auto operator-(const Year other) const noexcept -> Year {
        return Year{value_ + other.value_};
    }
    constexpr auto operator++() noexcept -> Year& {
        ++value_;
        return *this;
    }
};

class Date final {
    struct Normalized final {
        const Year  year;
        const Month month;
        const Day   day;
    };

  public:
    explicit constexpr Date(const int day) noexcept
        : Date{[day]() constexpr noexcept -> Normalized {
              const auto totalDays     = day - 1;
              const auto month         = totalDays / setting::dayInMonth;
              const auto normalizedDay = (totalDays % setting::dayInMonth) + 1;

              const auto totalMonth      = month - 1;
              const auto normalizedYear  = totalMonth / setting::monthInYear;
              const auto normalizedMonth = (totalMonth % setting::monthInYear) + 1;

              return {
                  .year  = Year{normalizedYear + 1},
                  .month = Month{normalizedMonth + 1},
                  .day   = Day{normalizedDay + 1}
              };
          }()} {}

    [[nodiscard]] constexpr auto isBeginingYear() const noexcept -> bool {
        return year_ == Year{1};
    }

    [[nodiscard]] constexpr auto day() const noexcept -> Day { return day_; }

    [[nodiscard]] constexpr auto month() const noexcept -> Month { return month_; }

    [[nodiscard]] constexpr auto toFlatTime() const noexcept -> Day {
        const auto yearIdx   = year_ - Year{1};
        const auto monthIdx  = month_ - Month{1};
        const auto dayIdx    = day_ - Day{1};
        const auto flatMonth = (yearIdx.value() * setting::monthInYear) + monthIdx.value();
        const auto flatDay   = (flatMonth * setting::dayInMonth) + dayIdx.value();
        return Day{flatDay};
    }

    [[nodiscard]] constexpr auto operator<=>(const Date&) const noexcept -> auto = default;

    constexpr auto operator++() noexcept -> Date& {
        if (day_ < Day{setting::dayInMonth}) {
            ++day_;
            return *this;
        }
        if (month_ < Month{setting::monthInYear}) {
            day_ = Day{1};
            ++month_;
            return *this;
        }
        day_   = Day{1};
        month_ = Month{1};
        ++year_;
        return *this;
    }

  private:
    explicit constexpr Date(const Normalized& normalized) noexcept
        : year_{normalized.year}, month_{normalized.month}, day_{normalized.day} {}

    Year  year_;
    Month month_;
    Day   day_;
};
}  // namespace abm