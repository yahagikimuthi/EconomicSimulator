#pragma once

#include "components/base_goods_demander.hpp"

namespace production_goods::demander {
class [[nodiscard]] ProductionGoodsDemander : public base_goods::demander::BaseGoodsDemander {
  public:
    using base_goods::demander::BaseGoodsDemander::BaseGoodsDemander;
};
}  // namespace production_goods::demander