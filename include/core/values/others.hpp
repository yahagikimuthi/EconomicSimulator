#pragma once

#include "core/assertion.hpp"
#include "core/values/common.hpp"

namespace abm {
class TaxRate : public value_object::BaseValueObjectMixin<double> {
  public:
    [[nodiscard]] explicit constexpr TaxRate(const double value)
        : BaseValueObjectMixin<double>(value) {
        ASSERT(0.0 <= value and value <= 1.0);
    }
};
}  // namespace abm