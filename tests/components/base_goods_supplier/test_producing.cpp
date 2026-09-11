#include "components/base_goods_supplier/produsing.hpp"

#include "doctest.h"
#include "tests/util.hpp"

namespace abm::base_goods::supplier {
namespace {
TEST_CASE("Producerのテスト") {  // NOLINT
    auto rng      = makeRng();
    auto producer = Producer{rng};

    SUBCASE("デフォルトで供給量はゼロ") { CHECK(producer.produce().isZero()); }
}
}  // namespace
}  // namespace abm::base_goods::supplier