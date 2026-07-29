#include <cstdint>
#include <pcg_random.hpp>
#include <world/message.hpp>

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
RequestPlanner::RequestPlanner(pcg32& masterRng)
    : rng_{makeSeed(masterRng), makeSeed(masterRng)},
      log_{
          .wage         = rand(masterRng, 1000, 5000),
          .actualEmploy = randInt(rng_, 4, 12),
          .offerPlan    = randInt(rng_, 10, 20),
          .applicantNum = randInt(masterRng, 10, 20)
      },
      param_{
          .offerRate      = rand(masterRng),
          .wageAdjustVol  = rand(masterRng, 0.01, 0.1),
          .offerAdjustVol = rand(masterRng, 0.3, 0.5)
      } {}

Recruiter::Recruiter(pcg32& masterRng) : planner_{masterRng} {}
HumanResourceManager::HumanResourceManager(world::CompanyBoard& companyBoard)
    : companyBoard_{companyBoard} {}

LaborDemander::LaborDemander(pcg32& masterRng, world::CompanyBoard& companyBoard)
    : recruiter_{masterRng}, hrManager_{companyBoard} {}
}  // namespace labor_demander
namespace labor_supplier {
Component::Component(const std::uint64_t state, const std::uint64_t stream)
    : rng_{state, stream},
      productPower_{randNormal(rng_, 1.0, 1.0 / 3.0, 0.0, 2.0)},
      jobSearchThreshold_{rand(rng_, 0.01, 0.05)} {}
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
Component::Component(
    const std::uint64_t state, const std::uint64_t stream, world::Workspace& workspace
)
    : rng_{state, stream},
      log_{
          .markup_         = rand(rng_, 0.1, 0.3),
          .supply_         = rand(rng_, 4, 15),
          .demandForecast_ = rand(rng_, 5.0, 20.0),
          .isSold_         = true
      },
      production_{
          .workspace_        = workspace,
          .firmProductPower_ = rand(rng_, 0.01, 0.05),
          .inventory_        = rand(rng_, 0.5, 2.0)
      },
      parameter_{
          .targetInventoryRatio_          = rand(rng_, 0.1, 0.2),
          .markupAdjustmentVolatility_    = rand(rng_, 0.01, 0.02),
          .demandForecastAdjustmentParam_ = rand(rng_, 0.1, 0.4)
      } {
    log_.isSold_                            = (rand(rng_) <= 0.5) ? true : false;
    production_.workspace_.firmProductPower = production_.firmProductPower_;
}
}  // namespace goods_supplier