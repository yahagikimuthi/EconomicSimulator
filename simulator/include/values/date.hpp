#pragma once

#include <compare>

#include "others/setting.hpp"
#include "values/mixin.hpp"

namespace abm::value_object {
class Day final : public BaseValueObject<int>, CompareMixin<Day>, AddMixin<Day> {
    friend struct AddMixin<Day>;

  public:
    explicit constexpr Day(const int value) noexcept : BaseValueObject<int>(value) {
        contract_assert(value > 0);
    }

    constexpr auto operator++() noexcept -> Day& {
        ++value_;
        return *this;
    }
};

class Month final : public BaseValueObject<int>, CompareMixin<Month>, AddMixin<Month> {
    friend struct AddMixin<Month>;

  public:
    explicit constexpr Month(const int value) noexcept : BaseValueObject<int>(value) {
        contract_assert(value > 0);
    }

    constexpr auto operator++() noexcept -> Month& {
        ++value_;
        return *this;
    }
};

class Year final : public BaseValueObject<int>, CompareMixin<Year>, AddMixin<Year> {
    friend struct AddMixin<Year>;

  public:
    explicit constexpr Year(const int value) noexcept : BaseValueObject<int>(value) {
        contract_assert(value > 0);
    }

    constexpr auto operator++() noexcept -> Year& {
        ++value_;
        return *this;
    }
};

class Date final {
    struct Normalized final {
        const int year;
        const int month;
        const int day;
    };

  public:
    explicit constexpr Date(const int day) noexcept
        : Date([day]() constexpr noexcept -> Normalized {
              contract_assert(day > 0);

              using namespace global_setting;
              constexpr auto dayInYear = dayInMonth * monthInYear;

              const int dayIdx          = day - 1;
              const int normalizedYear  = (dayIdx / dayInYear) + 1;
              const int remainder       = dayIdx % dayInYear;
              const int normalizedMonth = (remainder / dayInMonth) + 1;
              const int normalizedDay   = (remainder % dayInMonth) + 1;

              return {.year = normalizedYear, .month = normalizedMonth, .day = normalizedDay};
          }()) {}

    [[nodiscard]] constexpr auto year() const noexcept -> Year { return year_; }
    [[nodiscard]] constexpr auto month() const noexcept -> Month { return month_; }
    [[nodiscard]] constexpr auto day() const noexcept -> Day { return day_; }

    [[nodiscard]] constexpr auto operator<=>(const Date&) const noexcept -> auto = default;
    [[nodiscard]] constexpr auto operator==(const Date&) const noexcept -> bool  = default;

    constexpr auto operator++() noexcept -> Date& {
        if (day_ == Day{global_setting::dayInMonth}) {           // 月末で
            if (month_ == Month{global_setting::monthInYear}) {  // 年末のとき
                day_   = Day{1};
                month_ = Month{1};
                ++year_;
                return *this;
            }
            day_ = Day{1};  // 月末かつ年末でないとき
            ++month_;
            return *this;
        }
        ++day_;  // 月末でも何でもないとき
        return *this;
    }

  private:
    constexpr Date(const Normalized& normalized) noexcept
        : year_{normalized.year}, month_{normalized.month}, day_{normalized.day} {}

    Year  year_;
    Month month_;
    Day   day_;
};
}  // namespace abm::value_object

namespace abm {
using Day   = value_object::Day;
using Month = value_object::Month;
using Year  = value_object::Year;
using Date  = value_object::Date;
}  // namespace abm