#pragma once

#include <algorithm>

#include "config.hpp"
#include "core/base.hpp"

class [[nodiscard]] Price {
  public:
    explicit Price(const double values)
        : values_{std::min(config::goods_supplier::epsilonPrice, values)} {}
    auto value() const -> double { return values_; }
    auto operator<=>(const Price&) const = default;
    auto operator+=(const Price other) -> Price& {
        values_ += other.values_;
        return *this;
    }
    auto operator+(const Price other) const -> Price { return Price(values_ + other.values_); }
    auto operator-=(const Price other) -> Price& {
        values_ -= other.values_;
        return *this;
    }
    auto operator-(const Price other) const -> Price { return Price(values_ - other.values_); }

  private:
    double values_{};
};

class [[nodiscard]] GoodsQuantity {
  public:
    explicit GoodsQuantity(const double values) : values_{values} {}
    auto value() const -> double { return values_; }
    auto operator<=>(const GoodsQuantity&) const = default;
    auto operator+=(const GoodsQuantity other) -> GoodsQuantity& {
        values_ += other.values_;
        return *this;
    }
    auto operator+(const GoodsQuantity other) const -> GoodsQuantity {
        return GoodsQuantity(values_ + other.values_);
    }
    auto operator-=(const GoodsQuantity other) -> GoodsQuantity& {
        values_ -= other.values_;
        return *this;
    }
    auto operator-(const GoodsQuantity other) const -> GoodsQuantity {
        return GoodsQuantity(values_ - other.values_);
    }

  private:
    double values_;
};

class [[nodiscard]] Wage {
  public:
    explicit Wage(const double values)
        : values_{std::min(config::labor_demander::epsilonWage, values)} {}
    auto value() const -> double { return values_; }
    auto operator<=>(const Wage&) const = default;
    auto operator+=(const Wage other) -> Wage& {
        values_ += other.values_;
        return *this;
    }
    auto operator+(const Wage other) const -> Wage { return Wage(values_ + other.values_); }
    auto operator-=(const Wage other) -> Wage& {
        values_ -= other.values_;
        return *this;
    }
    auto operator-(const Wage other) const -> Wage { return Wage(values_ - other.values_); }

  private:
    double values_;
};

class [[nodiscard]] HeadCount {
  public:
    explicit HeadCount(const double values) : values_{values} {}
    auto value() const -> double { return values_; }
    auto operator<=>(const HeadCount&) const = default;
    auto operator+=(const HeadCount other) -> HeadCount& {
        values_ += other.values_;
        return *this;
    }
    auto operator+(const HeadCount other) const -> HeadCount {
        return HeadCount(values_ + other.values_);
    }
    auto operator-=(const HeadCount other) -> HeadCount& {
        values_ -= other.values_;
        return *this;
    }
    auto operator-(const HeadCount other) const -> HeadCount {
        return HeadCount(values_ - other.values_);
    }

  private:
    double values_;
};

class [[nodiscard]] AgentID {
  public:
    explicit AgentID(const int values) PRE(values >= 0.0) : values_{values} {}
    auto value() const -> int { return values_; }

  private:
    const int values_;
};

class [[nodiscard]] Money {
  public:
    explicit Money(const double values) : values_{values} {}
    auto value() const -> double { return values_; }
    auto operator<=>(const Money&) const = default;
    auto operator+=(const Money other) -> Money& {
        values_ += other.values_;
        return *this;
    }
    auto operator+(const Money other) const -> Money { return Money(values_ + other.values_); }
    auto operator-=(const Money other) -> Money& {
        values_ -= other.values_;
        return *this;
    }
    auto operator-(const Money other) const -> Money { return Money(values_ - other.values_); }

  private:
    double values_;
};

class [[nodiscard]] Step {
  public:
    explicit Step(const int step) PRE(step >= 0) : step_{step} {}
    auto value() const -> int { return static_cast<int>(step_); }
    auto operator<=>(const Step&) const = default;
    auto operator++() -> Step& {
        ++step_;
        return *this;
    }
    auto operator++(int) -> Step {
        Step temp{*this};
        ++(*this);
        return temp;
    }

  private:
    int step_;
};

/*
[[nodiscard]] inline auto operator*(const Price price, const GoodsQuantity quantity) -> Money {
    return Money{price.value() * quantity.value()};
}
[[nodiscard]] inline auto operator*(const Wage wage, const HeadCount headCount) -> Money {
    return Money{wage.value() * headCount.value()};
}
[[nodiscard]] inline auto operator/(const Money money, const GoodsQuantity quantity)
    PRE(quantity.value() != 0.0) -> Price {
    return Price{money.value() / quantity.value()};
}
[[nodiscard]] inline auto operator/(const Money money, const Price price)
    PRE(price.value() != 0.0) -> GoodsQuantity {
    return GoodsQuantity{money.value() / price.value()};
}
[[nodiscard]] inline auto operator*(const Money money, const double value) -> Money {
    return Money{money.value() * value};
}
[[nodiscard]] inline auto operator*(const double value, const Money money) -> Money {
    return money * value;
}
*/