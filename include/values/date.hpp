#pragma once

#include <compare>

#include "others/setting.hpp"
#include "others/util.hpp"

namespace abm {
class Day final {
  public:
    explicit constexpr Day(const int day) : day_{day} { ASSERT(day > 0); }

    [[nodiscard]] auto operator+(Day other) const noexcept -> Day { return Day{day_ + other.day_}; }
    [[nodiscard]] auto operator-(Day other) const noexcept -> Day { return Day{day_ - other.day_}; }
    [[nodiscard]] auto value() const noexcept -> int { return day_; }

    auto operator<=>(const Day&) const noexcept -> auto = default;
    auto operator==(const Day&) const noexcept -> bool  = default;

    auto operator++() noexcept -> Day& {
        ++day_;
        return *this;
    }

  private:
    int day_;
};

class Month final {
  public:
    explicit constexpr Month(const int month) : month_{month} { ASSERT(month > 0); }

    [[nodiscard]] auto value() const noexcept -> int { return month_; }

    auto operator<=>(const Month&) const noexcept -> auto = default;
    auto operator==(const Month&) const noexcept -> bool  = default;

    auto operator++() noexcept -> Month& {
        ++month_;
        return *this;
    }
    auto operator-(const Month other) const noexcept -> Month {
        return Month{month_ - other.month_};
    }

  private:
    int month_;
};

class Year final {
  public:
    explicit constexpr Year(const int year) : year_{year} { ASSERT(year > 0); }

    [[nodiscard]] auto operator-(const Year other) const noexcept -> Year {
        return Year{year_ + other.year_};
    }
    [[nodiscard]] auto value() const noexcept -> int { return year_; }

    auto operator<=>(const Year&) const noexcept -> auto = default;
    auto operator==(const Year&) const noexcept -> bool  = default;
    auto operator++() noexcept -> Year& {
        ++year_;
        return *this;
    }

  private:
    int year_;
};

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
    Year  year_;
    Month month_;
    Day   day_;
};
}  // namespace abm