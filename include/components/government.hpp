#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>

#include "components/finance/others_finance.hpp"
#include "others/setting.hpp"
#include "values/common.hpp"
#include "values/others.hpp"
#include "world/drop_box.hpp"

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
    explicit IncomeTaxStrategy() noexcept
        : FlatTaxStrategy::FlatTaxStrategy(TaxRate{setting::incomeTaxRate}) {}
};

class SalesTaxStrategy final : public FlatTaxStrategy {
  public:
    explicit SalesTaxStrategy() noexcept
        : FlatTaxStrategy::FlatTaxStrategy(TaxRate{setting::salesTaxRate}) {}
};

class CorporateTaxStrategy final : public FlatTaxStrategy {
  public:
    explicit CorporateTaxStrategy() noexcept
        : FlatTaxStrategy::FlatTaxStrategy(TaxRate{setting::corporateTaxRate}) {}
};

enum class TaxType : std::uint8_t { Income, Sales, Corporate };

class TaxStrategies final {
  public:
    explicit TaxStrategies() noexcept = default;

    [[nodiscard]] auto calc(const Money in, const TaxType type) noexcept -> Money {
        assert(in.isZeroOrMore());
        if (type == TaxType::Income) return income_.calculate(in);
        if (type == TaxType::Sales) return sales_.calculate(in);
        if (type == TaxType::Corporate) return corporate_.calculate(in);
        std::unreachable();
    }

  private:
    IncomeTaxStrategy    income_;
    SalesTaxStrategy     sales_;
    CorporateTaxStrategy corporate_;
};

class Government final {
  public:
    explicit Government() noexcept = default;

    [[nodiscard]] auto payIncomeTax(const Money income) noexcept -> Money {
        if (income.isZeroOrLess()) return income;
        const auto tax = taxStrategies_.calc(income, TaxType::Income);
        assert(tax <= income);
        assert(tax.isZeroOrMore());
        finance_.deposit(tax);
        return income - tax;
    }

    [[nodiscard]] auto paySalesTax(const Money sales) noexcept -> Money {
        if (sales.isZeroOrLess()) return sales;
        const auto tax = taxStrategies_.calc(sales, TaxType::Sales);
        assert(tax <= sales);
        assert(tax.isZeroOrMore());
        finance_.deposit(tax);
        return sales - tax;
    }

    [[nodiscard]] auto payCorporateTax(const Money profit) noexcept -> Money {
        if (profit.isZeroOrLess()) return profit;
        const auto tax = taxStrategies_.calc(profit, TaxType::Corporate);
        assert(tax <= profit);
        assert(tax.isZeroOrMore());
        finance_.deposit(tax);
        return profit - tax;
    }

    void setBudget(CensusDropBox& dropBox) noexcept {
        const auto unemployment = std::ranges::count_if(
            dropBox.labor.wages.get(), [](const double wage) -> bool { return wage == 0.0; }
        );
        const auto redCompanies = std::ranges::count_if(
            dropBox.finance.netIncome.get(),
            [](const double netIncome) -> bool { return netIncome <= 0.0; }
        );
        providePlan_ = finance_.asset() / static_cast<double>(unemployment + redCompanies);
    }

    [[nodiscard]] auto provideUnemploymentBenefit() noexcept -> Money {
        assert(providePlan_);
        const auto withdraw = finance_.tryWithdraw(*providePlan_);
        return withdraw;
    }

    [[nodiscard]] auto subsideLossMakingCompany() noexcept -> Money {
        assert(providePlan_);
        const auto withdraw = finance_.tryWithdraw(*providePlan_);
        return withdraw;
    }

    void reset() noexcept { providePlan_.reset(); }

    [[nodiscard]] auto asset() const noexcept -> Budget { return finance_.asset(); }

  private:
    GovernmentFinance     finance_;
    TaxStrategies         taxStrategies_;
    std::optional<Budget> providePlan_;
};
}  // namespace abm::government

namespace abm {
using Government = government::Government;
}