#pragma once

#include <span>

#include "components/government.hpp"
#include "engine/agents.hpp"
#include "system/capital.hpp"
#include "world/base_goods.hpp"
#include "world/drop_box.hpp"

namespace abm::engine {
class CapitalEngine final {
  public:
    explicit CapitalEngine(CapitalDropBox& dropBox) noexcept : dropBox_{dropBox} {}

    void run(std::span<CapitalFirm> capital, std::span<GoodsFirm> goods, Government& gov) noexcept {
        using namespace capital;
        for (auto& firm : capital) {
            entry(firm.id, firm.capitalSupplier, market_);
        }

        for (auto& firm : capital) {
            request(firm.id, firm.finance, firm.capitalDemander, market_);
        }
        for (auto& firm : goods) {
            request(firm.id, firm.finance, firm.capitalDemander, market_);
        }

        for (auto& firm : capital) {
            trade(firm.finance, firm.capitalSupplier, gov);
        }

        for (auto& firm : capital) {
            afterTrade(firm.finance, firm.capitalDemander, firm.capitalSupplier);
        }
        for (auto& firm : goods) {
            afterTrade(firm.finance, firm.capitalDemander, firm.goodsSupplier);
        }

        market_.clear();

        for (auto& firm : capital) {
            logging(dropBox_, firm.capitalSupplier);
        }
    }

  private:
    CapitalMarket   market_;
    CapitalDropBox& dropBox_;
};
}  // namespace abm::engine