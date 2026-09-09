#pragma once

#include <cassert>

#include "components/finance/others_finance.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
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
    explicit Government(RandomGenerator& masterRng) noexcept
        : finance_{masterRng},
          incomeTaxStrategy_{TaxRate{setting::incomeTaxRate}},
          salesTaxStrategy_{TaxRate{setting::salesTaxRate}},
          corporateTaxStrategy_{TaxRate{setting::corporateTaxRate}} {}

    [[nodiscard]] auto collectIncomeTax(const Money income) noexcept -> Money {
        if (income <= Money{0.0}) return income;
        const auto tax = incomeTaxStrategy_.calculate(income);
        assert(tax <= income);
        finance_.deposit(tax);
        return income - tax;
    }

    [[nodiscard]] auto collectSalesTax(const Money sales) noexcept -> Money {
        if (sales <= Money{0.0}) return sales;
        const auto tax = salesTaxStrategy_.calculate(sales);
        assert(tax <= sales);
        finance_.deposit(tax);
        return sales - tax;
    }

    [[nodiscard]] auto collectCorporateTax(const Money profit) noexcept -> Money {
        if (profit <= Money{0.0}) return profit;
        const auto tax = corporateTaxStrategy_.calculate(profit);
        assert(tax <= profit);
        finance_.deposit(tax);
        return profit - tax;
    }

    [[nodiscard]] auto provideUnemploymentBenefit(const Wage wage) noexcept -> Money {
        if (wage.isPositive()) return Money{0.0};
        if (finance_.asset().isZeroOrLess()) return Money{0.0};
        ++nextUnemploymentHHoldCnt_;
        const auto cnt     = (lastUnemploymentHHoldCnt_ != 0) ? lastUnemploymentHHoldCnt_ : 1;
        const auto provide = finance_.asset() / cnt;
        const auto out     = finance_.tryWithdraw(provide);
        return out;
    }

    void reset() noexcept {
        lastUnemploymentHHoldCnt_ = nextUnemploymentHHoldCnt_;
        nextUnemploymentHHoldCnt_ = 0;
    }

    [[nodiscard]] auto asset() const noexcept -> Budget { return finance_.asset(); }

  private:
    GovernmentFinance    finance_;
    IncomeTaxStrategy    incomeTaxStrategy_;
    SalesTaxStrategy     salesTaxStrategy_;
    CorporateTaxStrategy corporateTaxStrategy_;
    int                  lastUnemploymentHHoldCnt_{};
    int                  nextUnemploymentHHoldCnt_{};
};
}  // namespace abm::government

namespace abm {
using Government = government::Government;
}