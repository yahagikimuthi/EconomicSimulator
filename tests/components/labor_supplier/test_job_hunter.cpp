#include "components/labor_supplier/job_hunter.hpp"

#include "components/finance/others_finance.hpp"
#include "components/labor_supplier/employment.hpp"
#include "doctest.h"
#include "tests/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
namespace {
TEST_CASE("JobHunterのテスト") {  // NOLINT
    constexpr auto id = AgentID{42};

    auto                   rng         = makeRng();
    auto                   hunter      = JobHunter{rng};
    auto                   employment  = Employment{rng};
    auto                   board       = CompanyBoard{AgentID{101}, Day{15}};
    auto                   space       = base_goods::Workspace{};
    auto                   roster      = Roster{};
    [[maybe_unused]] auto& rosterEntry = roster.add(id, Wage{10}, board, space);
    auto                   market      = Market{};
    auto                   finance     = HHoldFinance{id, rng};

    SUBCASE("何もポストされていない場合、resultは空") {
        hunter.entry(
            id, employment.makeIsAlignedRequestFn(), employment.makeEntrySheetFn(id), market
        );
        hunter.accept();
        const auto result = hunter.huntedResult();

        CHECK(not result);
    }

    SUBCASE("就職している場合でも、何もポストされていない場合は空") {
        employment.startWorking(rosterEntry, finance.makeDepositFn());

        hunter.entry(
            id, employment.makeIsAlignedRequestFn(), employment.makeEntrySheetFn(id), market
        );

        hunter.accept();
        const auto result = hunter.huntedResult();

        CHECK(not result);
    }

    SUBCASE("同じ企業のリクエストのみ存在する場合、結果は空") {
        employment.startWorking(rosterEntry, finance.makeDepositFn());

        auto& req = market.request(AgentID{101}, Wage{1000});

        hunter.entry(
            id, employment.makeIsAlignedRequestFn(), employment.makeEntrySheetFn(id), market
        );

        CHECK(req.entries().empty());

        hunter.accept();

        const auto result = hunter.huntedResult();

        CHECK(not result);
    }

    SUBCASE("賃金が低いリクエストのみ存在する場合、結果は空") {
        employment.startWorking(rosterEntry, finance.makeDepositFn());

        auto& req = market.request(AgentID{202}, Wage{1});

        hunter.entry(
            id, employment.makeIsAlignedRequestFn(), employment.makeEntrySheetFn(id), market
        );

        CHECK(req.entries().empty());

        hunter.accept();

        const auto result = hunter.huntedResult();

        CHECK(not result);
    }
}
}  // namespace
}  // namespace abm::labor::supplier