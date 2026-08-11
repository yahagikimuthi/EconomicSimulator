#pragma once

#include "components/base_goods_demander.hpp"

namespace consumer_goods::demander {
class [[nodiscard]] ConsumerGoodsDemander : public base_goods::demander::BaseGoodsDemander {
  public:
    using base_goods::demander::BaseGoodsDemander::BaseGoodsDemander;
};
}  // namespace consumer_goods::demander