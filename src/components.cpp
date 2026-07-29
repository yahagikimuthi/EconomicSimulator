#include <pcg_random.hpp>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "config.hpp"
#include "helper.hpp"
#include "world/message.hpp"

using namespace helper;

namespace firm_finance {
Component::Component(pcg32& masterRng) : asset_{rand(masterRng, 100000, 500000)} {}
}  // namespace firm_finance
namespace hhold_finance {
Component::Component(pcg32& masterRng) : asset_{rand(masterRng, 100000, 500000)} {}
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
JobHunter ::JobHunter(pcg32& masterRng) : rng_{makeSeed(masterRng), makeSeed(masterRng)} {}
LaborSupplier::LaborSupplier(pcg32& masterRng)
    : rng_{makeSeed(masterRng), makeSeed(masterRng)},
      jobHunter_{masterRng},
      productPower_{randNormal(masterRng, 1.0, 1.0 / 3.0, 0.0, 2.0)},
      jobSearchThreshold_{rand(masterRng, 0.01, 0.05)} {}
}  // namespace labor_supplier

namespace goods_demander {
GoodsDemander::GoodsDemander(pcg32& masterRng)
    : rng_{makeSeed(masterRng), makeSeed(masterRng)},
      mpc_{rand(rng_, 0.7, 0.9)},
      myPhase_{instanceCnt_++ % config::goods_demander::maxPurchaseFrequency} {}
}  // namespace goods_demander

namespace goods_supplier {
Planner::Planner(pcg32& masterRng)
    : rng_{makeSeed(masterRng), makeSeed(masterRng)},
      log_{
          .markup         = rand(masterRng, 0.1, 0.3),
          .supply         = rand(masterRng, 4, 15),
          .demandForecast = rand(masterRng, 5.0, 20.0),
          .isSold         = rand(masterRng) < 0.5
      },
      param_{
          .targetInvRatio          = rand(masterRng, 0.1, 0.2),
          .markupAdjustVol         = rand(masterRng, 0.01, 0.02),
          .demandForecastAdjustVol = rand(masterRng, 0.1, 0.4)
      } {}

Trader::Trader(pcg32& masterRng) : rng_{makeSeed(masterRng), makeSeed(masterRng)} {}
Producer::Producer(pcg32& masterRng, world::Workspace& workspace)
    : workspace_{workspace},
      firmProductPower_{rand(masterRng, 0.01, 0.05)},
      inventory_{rand(masterRng, 0.5, 2.0)} {
    workspace_.firmProductPower = firmProductPower_;
}
GoodsSupplier::GoodsSupplier(pcg32& masterRng, world::Workspace& workspace)
    : planner_{masterRng}, trader_{masterRng}, producer_{masterRng, workspace} {}
}  // namespace goods_supplier