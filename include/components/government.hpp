#pragma once

#include <cassert>

#include "components/finance/others_finance.hpp"
#include "others/setting.hpp"
#include "values/common.hpp"
#include "values/others.hpp"

namespace abm::government {
class FlatTaxStrategy {
  public:
    [[nodiscard]] auto calculate(const Money amount) const noexcept -> Money {
        return amount * rate_;
    }

  protected:
    explicit FlatTaxStrategy(const TaxRate rate) noexcept : rate_{rate} {}

  private:
    TaxRate rate_;
};

class IncomeTaxStrategy final : public FlatTaxStrategy {
  public:
    explicit IncomeTaxStrategy(const TaxRate rate) noexcept
        : FlatTaxStrategy::FlatTaxStrategy(rate) {}
};

class SalesTaxStrategy final : public FlatTaxStrategy {
  public:
    explicit SalesTaxStrategy(const TaxRate rate) noexcept
        : FlatTaxStrategy::FlatTaxStrategy(rate) {}
};

class CorporateTaxStrategy final : public FlatTaxStrategy {
  public:
    explicit CorporateTaxStrategy(const TaxRate rate) noexcept
        : FlatTaxStrategy::FlatTaxStrategy(rate) {}
};

class Government final {
  public:
    explicit Government() noexcept
        : incomeTaxStrategy_{TaxRate{setting::incomeTaxRate}},
          salesTaxStrategy_{TaxRate{setting::salesTaxRate}},
          corporateTaxStrategy_{TaxRate{setting::corporateTaxRate}} {}

    [[nodiscard]] auto payIncomeTax(const Money income) noexcept -> Money {
        if (income.isZeroOrLess()) return income;
        const auto tax = incomeTaxStrategy_.calculate(income);
        assert(tax <= income);
        assert(tax.isZeroOrMore());
        finance_.deposit(tax);
        return income - tax;
    }

    [[nodiscard]] auto paySalesTax(const Money sales) noexcept -> Money {
        if (sales.isZeroOrLess()) return sales;
        const auto tax = salesTaxStrategy_.calculate(sales);
        assert(tax <= sales);
        assert(tax.isZeroOrMore());
        finance_.deposit(tax);
        return sales - tax;
    }

    [[nodiscard]] auto payCorporateTax(const Money profit) noexcept -> Money {
        if (profit.isZeroOrLess()) return profit;
        const auto tax = corporateTaxStrategy_.calculate(profit);
        assert(tax <= profit);
        assert(tax.isZeroOrMore());
        finance_.deposit(tax);
        return profit - tax;
    }

    [[nodiscard]] auto provideUnemploymentBenefit(const Wage wage) noexcept -> Money;

    [[nodiscard]] auto subsideLossMakingCompany(const Money profit) noexcept -> Money;

    [[nodiscard]] auto asset() const noexcept -> Budget { return finance_.asset(); }

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