#pragma once

#include "core/values/common.hpp"

namespace abm {
class TaxRate : public value_object::ValueObjectMixin<TaxRate> {
  public:
    [[nodiscard]] explicit constexpr TaxRate(const double value)
        : ValueObjectMixin<TaxRate>(value) {}
};
}  // namespace abm