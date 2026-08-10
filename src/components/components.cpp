#include <core/values/goods.hpp>
#include <pcg_random.hpp>

#include "components/goods_supplier.hpp"
#include "components/labor_supplier/labor_supplier.hpp"

using namespace helper;

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