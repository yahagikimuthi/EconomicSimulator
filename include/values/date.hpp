#pragma once

#include <compare>

namespace abm {
class Date final {
  public:
    [[nodiscard]] constexpr Date(
        const unsigned int year, const unsigned int month, const unsigned int day
    ) noexcept
        : year_{year}, month_{month}, day_{day} {}

    [[nodiscard]] auto operator<=>(const Date&) const noexcept -> auto = default;
    [[nodiscard]] auto operator==(const Date&) const noexcept -> bool  = default;

  private:
    unsigned int year_;
    unsigned int month_;
    unsigned int day_;
};
}  // namespace abm