#include "components/labor_demander/labor_demander.hpp"

#include "components/labor_demander/mediator.hpp"
#include "doctest.h"
#include "tests/util.hpp"

namespace abm::labor::demander {
namespace {
TEST_CASE("RecruitSystemのテスト") {  // NOLINT
    auto rng      = makeRng();
    auto system   = RecruitSystem{rng};
    auto mediator = Mediator{};
    system.acceptMediator(mediator);
}
}  // namespace
}  // namespace abm::labor::demander