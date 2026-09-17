#pragma once

#include <compare>

#include "others/setting.hpp"
#include "others/type.hpp"
#include "values/mixin.hpp"

namespace abm::value_object {
class Day final : public BaseValueObject<i32>, CompareMixin<Day>, AddMixin<Day> {
    friend struct AddMixin<Day>;

  public:
    explicit constexpr Day(const i32 value) noexcept : BaseValueObject<i32>(value) {
        contract_assert(value > 0);
    }

    constexpr auto operator++() noexcept -> Day& {
        ++value_;
        return *this;
    }
};

class Month final : public BaseValueObject<i32>, CompareMixin<Month>, AddMixin<Month> {
    friend struct AddMixin<Month>;

  public:
    explicit constexpr Month(const i32 value) noexcept : BaseValueObject<i32>(value) {
        contract_assert(value > 0);
    }

    constexpr auto operator++() noexcept -> Month& {
        ++value_;
        return *this;
    }
};

class Year final : public BaseValueObject<i32>, CompareMixin<Year>, AddMixin<Year> {
    friend struct AddMixin<Year>;

  public:
    explicit constexpr Year(const i32 value) noexcept : BaseValueObject<i32>(value) {
        contract_assert(value > 0);
    }

    constexpr auto operator++() noexcept -> Year& {
        ++value_;
        return *this;
    }
};

class Date final {
    struct Normalized final {
        const i32 year;
        const i32 month;
        const i32 day;
    };

  public:
    explicit constexpr Date(const i32 day) noexcept
        : Date([day]() constexpr noexcept -> Normalized {
              contract_assert(day > 0);

              using namespace global_setting;
              constexpr auto dayInYear = dayInMonth * monthInYear;

              const i32 dayIdx          = day - 1;
              const i32 normalizedYear  = (dayIdx / dayInYear) + 1;
              const i32 remainder       = dayIdx % dayInYear;
              const i32 normalizedMonth = (remainder / dayInMonth) + 1;
              const i32 normalizedDay   = (remainder % dayInMonth) + 1;

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