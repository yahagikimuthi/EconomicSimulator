#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>
#include <type_traits>

#include "components/base_concepts.hpp"
#include "core/base.hpp"
#include "core/values/common.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace base_goods::demander::internal {
template <EntryType Entry>
[[nodiscard]] inline auto pickEntry(
    pcg32&                         rng,
    tbb::concurrent_vector<Entry>& entryBox,
    const int                      sampleCnt = config::goods_demander::goodsSampleCnt
) -> Entry& {
    auto toDouble{[](const Entry& entry) -> double { return entry.supply.value(); }};
    std::reference_wrapper<Entry> betterEntry =
        helper::discreteDistribution(entryBox, rng, toDouble);

    if (sampleCnt <= 1) return betterEntry.get();

    for (const auto _ : std::views::iota(0, sampleCnt - 1)) {
        auto& sampleEntry = helper::discreteDistribution(entryBox, rng, toDouble);
        if (betterEntry.get().price <= sampleEntry.price) continue;
        betterEntry = std::ref(sampleEntry);
    }
    return betterEntry.get();
}
}  // namespace base_goods::demander::internal

namespace base_goods::demander {

template <FirmType T>
class [[nodiscard]] BaseGoodsDemander {
    using Request = std::
        conditional_t<T == FirmType::consumerGoods, ConsumerGoodsRequest, ProductionGoodsRequest>;

  public:
    BaseGoodsDemander(const pcg32 rng, const double mpc) : rng_{rng}, mpc_{mpc} {}

    void afterTrade() {
        if (not isPosting_) return;
        purchasing_ += myRequest_->entry.price * myRequest_->tradeAmount;
    }

    void endStep() {
        myRequest_.reset();
        isPosting_  = false;
        purchasing_ = Money{0.0};
    }

    auto purchase() const -> Money POST(money : money >= Money{0.0}) { return purchasing_; }

  protected:
    auto calcBudget(const Money asset) const -> Money { return asset * mpc_; }

    pcg32                         rng_;
    std::optional<const Request&> myRequest_{std::nullopt};
    bool                          isPosting_{false};
    Money                         purchasing_{0.0};
    const double                  mpc_;
};
}  // namespace base_goods::demander