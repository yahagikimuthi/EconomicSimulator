#include <core/values/goods.hpp>
#include <pcg_random.hpp>

#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"

using namespace helper;

namespace labor::demander {
RequestPlanner::RequestPlanner(
    const pcg32     rng,
    const Wage      lastWage,
    const HeadCount lastEmploy,
    const HeadCount lastOfferPlan,
    const HeadCount lastApplicantNum,
    const double    offerRate,
    const double    wageAdjustVol,
    const double    offerAdjustVol
)
    : rng_{rng},
      log_{
          .wage         = lastWage,
          .actualEmploy = lastEmploy,
          .offerPlan    = lastOfferPlan,
          .applicantNum = lastApplicantNum
      },
      param_{
          .offerRate = offerRate, .wageAdjustVol = wageAdjustVol, .offerAdjustVol = offerAdjustVol
      } {}
}  // namespace labor::demander

namespace labor::supplier {
LaborSupplier::LaborSupplier(
    const pcg32        rng,
    const JobHunter&&  jobHunter,
    const Employment&& employment,
    const double       jobSearchThreshold
)
    : rng_{rng},
      jobHunter_{jobHunter},
      employment_{employment},
      jobSearchThreshold_{jobSearchThreshold} {}
}  // namespace labor::supplier

namespace goods::supplier {
Planner::Planner(
    const pcg32         rng,
    const double        lastMarkup,
    const GoodsQuantity lastSupply,
    const GoodsQuantity demandForecast,
    const bool          isSold,
    const double        targetInvRatio,
    const double        markupAdjustVol,
    const double        demandForecastAdjustVol
)
    : rng_{rng},
      log_{
          .markup         = lastMarkup,
          .supply         = lastSupply,
          .demandForecast = demandForecast,
          .isSold         = isSold
      },
      param_{
          .targetInvRatio          = targetInvRatio,
          .markupAdjustVol         = markupAdjustVol,
          .demandForecastAdjustVol = demandForecastAdjustVol
      } {}
}  // namespace goods::supplier