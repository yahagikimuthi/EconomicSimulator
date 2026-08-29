#pragma once

#include "components/finance/finance.hpp"
#include "others/setting.hpp"
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
    [[nodiscard]] explicit constexpr FlatTaxStrategy(const TaxRate rate) noexcept : rate_{rate} {}

  private:
    TaxRate rate_;
};

class IncomeTaxStrategy final : public FlatTaxStrategy {
  public:
    [[nodiscard]] explicit constexpr IncomeTaxStrategy(const TaxRate rate) noexcept
        : FlatTaxStrategy::FlatTaxStrategy(rate) {}
};

class SalesTaxStrategy final : public FlatTaxStrategy {
  public:
    [[nodiscard]] explicit constexpr SalesTaxStrategy(const TaxRate rate) noexcept
        : FlatTaxStrategy::FlatTaxStrategy(rate) {}
};

class CorporateTaxStrategy final : public FlatTaxStrategy {
  public:
    [[nodiscard]] explicit constexpr CorporateTaxStrategy(const TaxRate rate) noexcept
        : FlatTaxStrategy::FlatTaxStrategy(rate) {}
};

class Government final {
  public:
    [[nodiscard]] explicit constexpr Government() noexcept
        : incomeTaxStrategy_{TaxRate{setting::incomeTaxRate}},
          salesTaxStrategy_{TaxRate{setting::salesTaxRate}},
          corporateTaxStrategy_{TaxRate{setting::corporateTaxRate}} {}

    [[nodiscard]] auto collectIncomeTax(const Money income) noexcept -> Money {
        if (income <= Money{0.0}) return income;
        const auto tax = incomeTaxStrategy_.calculate(income);
        ASSERT(tax <= income);
        finance_.assetPlus(tax);
        return income - tax;
    }

    [[nodiscard]] auto collectSalesTax(const Money sales) noexcept -> Money {
        if (sales <= Money{0.0}) return sales;
        const auto tax = salesTaxStrategy_.calculate(sales);
        ASSERT(tax <= sales);
        finance_.assetPlus(tax);
        return sales - tax;
    }

    [[nodiscard]] auto collectCorporateTax(const Money profit) noexcept -> Money {
        if (profit <= Money{0.0}) return profit;
        const auto tax = corporateTaxStrategy_.calculate(profit);
        ASSERT(tax <= profit);
        finance_.assetPlus(tax);
        return profit - tax;
    }

    [[nodiscard]] auto provideUnemploymentBenefit(const Wage wage) noexcept -> Money {
        if (wage > Wage{0.0}) return Money{0.0};
        if (finance_.asset() <= Money{0.0}) return Money{0.0};
        ++nextUnemploymentHHoldCnt_;
        const auto cnt     = (lastUnemploymentHHoldCnt_ != 0) ? lastUnemploymentHHoldCnt_ : 1;
        const auto provide = finance_.asset() / cnt;
        finance_.assetPlus(-provide);
        return provide;
    }

    void reset() noexcept {
        lastUnemploymentHHoldCnt_ = nextUnemploymentHHoldCnt_;
        nextUnemploymentHHoldCnt_ = 0;
    }

    [[nodiscard]] auto asset() const noexcept -> Money { return finance_.asset(); }

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