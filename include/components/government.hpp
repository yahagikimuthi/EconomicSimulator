#pragma once

#include "components/common.hpp"
#include "core/values/common.hpp"

namespace abm::government {

class FlatTaxStrategy {
  public:
    [[nodiscard]] auto calculate(const Money amount) const noexcept -> Money {
        return amount * rate_;
    }

  protected:
    [[nodiscard]] explicit constexpr FlatTaxStrategy(const double rate) noexcept : rate_{rate} {}

  private:
    double rate_;
};

class IncomeTaxStrategy : public FlatTaxStrategy {
  public:
    [[nodiscard]] explicit constexpr IncomeTaxStrategy(const double rate) noexcept
        : FlatTaxStrategy::FlatTaxStrategy(rate) {}
};

class SalesTaxStrategy : public FlatTaxStrategy {
  public:
    [[nodiscard]] explicit constexpr SalesTaxStrategy(const double rate) noexcept
        : FlatTaxStrategy::FlatTaxStrategy(rate) {}
};

class CorporateTaxStrategy final : public FlatTaxStrategy {
  public:
    [[nodiscard]] explicit constexpr CorporateTaxStrategy(const double rate) noexcept
        : FlatTaxStrategy::FlatTaxStrategy(rate) {}
};

class Government final {
  public:
    [[nodiscard]] explicit constexpr Government();

    [[nodiscard]] auto collectIncomeTax(const Money income) noexcept -> Money {
        const auto tax = incomeTaxStrategy_.calculate(income);
        ASSERT(tax <= income);
        finance_.assetPlus(tax);
        return income - tax;
    }

    [[nodiscard]] auto collectSalesTax(const Money sales) noexcept -> Money {
        const auto tax = salesTaxStrategy_.calculate(sales);
        ASSERT(tax <= sales);
        finance_.assetPlus(tax);
        return sales - tax;
    }

    [[nodiscard]] auto collectCorporateTax(const Money profit) noexcept -> Money {
        const auto tax = corporateTaxStrategy_.calculate(profit);
        ASSERT(tax <= profit);
        finance_.assetPlus(tax);
        return profit - tax;
    }

  private:
    GovernmentFinance    finance_;
    IncomeTaxStrategy    incomeTaxStrategy_;
    SalesTaxStrategy     salesTaxStrategy_;
    CorporateTaxStrategy corporateTaxStrategy_;
};
}  // namespace abm::government

namespace abm {
using Government = government::Government;
}