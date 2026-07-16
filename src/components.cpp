#include <cstdint>
#include <pcg_random.hpp>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "config.hpp"
#include "helper.hpp"

using namespace helper;

namespace firm_finance {
Component::Component(const std::uint64_t state, const std::uint64_t stream)
    : rng_{state, stream}, asset_{rand(rng_, 100000, 500000)} {}
}  // namespace firm_finance
namespace hhold_finance {
Component::Component(const std::uint64_t state, const std::uint64_t stream)
    : rng_{state, stream}, asset_{rand(rng_, 100000, 500000)} {}
}  // namespace hhold_finance

namespace labor_demander {
Component::Component(const std::uint64_t state, const std::uint64_t stream)
    : rng_{state, stream},
      log_{
          .wage_         = rand(rng_, 1000, 5000),
          .actualEmploy_ = randInt(rng_, 4, 12),
          .offerPlan_    = randInt(rng_, 10, 20),
          .applicantNum_ = randInt(rng_, 10, 20)
      },
      parameter_{
          .offerRate_                  = rand(rng_, 0.0, 1.0),
          .wageAdjustmentVolatility_   = rand(rng_, 0.01, 0.1),
          .employAdjustmentVolatility_ = rand(rng_, 1, 3),
          .offerAdjustmentVolatility_  = rand(rng_, 0.3, 0.5)
      } {
    log_.actualEmploy_ = std::max(log_.actualEmploy_, log_.offerPlan_);
    log_.applicantNum_ = std::max(log_.applicantNum_, log_.actualEmploy_);
}
}  // namespace labor_demander
namespace labor_supplier {
Component::Component(const std::uint64_t state, const std::uint64_t stream)
    : rng_{state, stream},
      parameter_{.productPower_ = randNormal(rng_, 1.0, 1.0 / 3.0, 0.0, 2.0)} {}
}  // namespace labor_supplier

namespace goods_demander {
Component::Component(const std::uint64_t state, const std::uint64_t stream)
    : rng_{state, stream},
      parameter_{
          .mpc_     = rand(rng_, 0.7, 0.9),
          .myPhase_ = instanceCnt_++ % config::goods_demander::maxPurchaseFrequency
      } {}
}  // namespace goods_demander

namespace goods_supplier {
Component::Component(const std::uint64_t state, const std::uint64_t stream)
    : rng_{state, stream},
      log_{
          .markup_ = rand(rng_, 0.1, 0.3),
          .price_  = rand(rng_, 1.0, 1.2),
          .supply_ = rand(rng_, 4.0, 12.0),
          .sales_  = rand(rng_, 4.0, 14.0),
          .isSold_ = true
      },
      production_{
          .firmProductPower_        = rand(rng_, 0.0001, 0.0005),
          .sumEmployeeProductPower_ = 0.0,
          .inventory_               = rand(rng_, 0.5, 2.0)
      },
      parameter_{
          .targetInventoryRatio_       = rand(rng_, 0.1, 0.2),
          .markupAdjustmentVolatility_ = rand(rng_, 0.01, 0.02)
      } {
    log_.isSold_ =
        log_.price_ * production_.inventory_ / log_.supply_ < parameter_.targetInventoryRatio_;
    log_.supply_ = std::min(log_.supply_, production_.inventory_);
    log_.sales_  = log_.price_ * (log_.supply_ - production_.inventory_);
}
}  // namespace goods_supplier