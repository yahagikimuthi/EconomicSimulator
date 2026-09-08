#include "world/labor.hpp"

#include <algorithm>
#include <array>
#include <inplace_vector>
#include <ranges>
#include <span>
#include <utility>

#include "doctest.h"
#include "others/util.hpp"
#include "tests/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"

namespace abm::labor {
namespace {
TEST_CASE("Rosterのテスト") {  // NOLINT
    auto roster = Roster{};
    auto board  = CompanyBoard{AgentID{42}, Day{1}};
    auto space  = base_goods::Workspace{};

    SUBCASE("addのみの場合、sumWageが機能することのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        nothing(roster.add(AgentID{202}, Wage{202}, board, space));
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        CHECK(roster.sumWage().value() == doctest::Approx(606));
    }

    SUBCASE("名簿を削除する場合もsumWageが機能することのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        auto& disableEntry = roster.add(AgentID{202}, Wage{202}, board, space);
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        disableEntry.resign();

        CHECK(roster.sumWage().value() == doctest::Approx(404));
    }

    SUBCASE("名簿が再利用されていることのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        auto* const beforeEntry = &roster.add(AgentID{202}, Wage{202}, board, space);
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        beforeEntry->resign();

        auto* const afterEntry = &roster.add(AgentID{404}, Wage{404}, board, space);

        CHECK(beforeEntry == afterEntry);
    }

    SUBCASE("validEntriesがすべて有効であることのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        auto& disableEntry = roster.add(AgentID{202}, Wage{202}, board, space);
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        disableEntry.resign();

        for (const auto& entry : std::as_const(roster).validEntries()) CHECK(entry.isOccupied());
    }

    SUBCASE("employeeCntのテスト") {
        nothing(roster.add(AgentID{101}, Wage{101}, board, space));
        auto& disableEntry = roster.add(AgentID{202}, Wage{202}, board, space);
        nothing(roster.add(AgentID{303}, Wage{303}, board, space));

        CHECK(roster.employeeCnt().value() == 3);

        disableEntry.resign();

        CHECK(roster.employeeCnt().value() == 2);
    }
}

TEST_CASE("RosterEntryのテスト") {  // NOLINT
    constexpr auto workDay = Day{15};

    auto  roster = Roster{};
    auto  board  = CompanyBoard{AgentID{42}, workDay};
    auto  space  = base_goods::Workspace{};
    auto& entry  = roster.add(AgentID{101}, Wage{101}, board, space);

    SUBCASE("労働日の場合、Workspaceに労働貢献を実際に行うこと") {
        entry.addInput(10.0, Date{workDay.value() - 1});

        const auto input = space.takeout();
        CHECK(input.value() == 10.0);
    }

    SUBCASE("労働日でない場合、労働貢献は事実上行わないこと") {
        entry.addInput(10.0, Date{2});

        const auto input = space.takeout();
        CHECK(input.isZero());
    }

    SUBCASE("resignが呼ばれた場合、isOccupiedがfalseとなること") {
        entry.resign();

        CHECK(not entry.isOccupied());
    }

    SUBCASE("takeoutは何もしない場合0が戻り値") { CHECK(entry.takeoutPaidWage().isZero()); }

    SUBCASE("給与を振り込んだ場合、takeoutを行うと同じものが返り、2回目は0が返る") {
        constexpr auto payment = Money{100.0};
        entry.payWage(payment);

        const auto takeout = entry.takeoutPaidWage();

        CHECK(takeout.value() == doctest::Approx(payment.value()));

        CHECK(entry.takeoutPaidWage().isZero());
    }
}

TEST_CASE("Marketのテスト") {  // NOLINT
    auto market = Market{};

    const auto requests = std::array{
        std::pair{AgentID{101}, Wage{101}},
        std::pair{AgentID{202}, Wage{202}},
        std::pair{AgentID{303}, Wage{303}}
    };
    auto out = std::inplace_vector<Ref<Request>, 3UZ>{};
    auto rng = makeRng();

    SUBCASE("何もしない場合、outは空") {
        market.pickRequest(AgentID{42}, out, rng);
        CHECK(out.empty());
    }

    SUBCASE("ID,Wageが恒等であることのテスト") {
        const auto id   = rng.randInt(0, 10000);
        const auto wage = rng.rand(0.0, 100000.0);

        const auto& req = market.request(AgentID{id}, Wage{wage});
        CHECK(req.firmID.value() == id);
        CHECK(req.wage.value() == doctest::Approx(wage));

        market.pickRequest(AgentID{-1}, out, rng);

        CHECK(out.size() == 1UZ);
        const auto& sample = out[0].get();
        CHECK(sample.firmID.value() == id);
        CHECK(req.wage.value() == doctest::Approx(wage));
    }

    SUBCASE("requestの数が引き出す数以下の場合、すべてをピックする") {
        for (auto [id, wage] : requests) nothing(market.request(id, wage));

        market.pickRequest(AgentID{42}, out, rng);

        const auto sumID = std::ranges::fold_left(
            out | std::views::transform(&Request::firmID) | std::views::transform(&AgentID::value),
            0.0,
            std::plus{}
        );
        CHECK(out.size() == 3UZ);
        CHECK(sumID == 606);
    }

    SUBCASE("IDが重複している場合、それは除かれる") {
        for (auto [id, wage] : requests) nothing(market.request(id, wage));

        market.pickRequest(AgentID{101}, out, rng);

        const auto sumID = std::ranges::fold_left(
            out | std::views::transform(&Request::firmID) | std::views::transform(&AgentID::value),
            0.0,
            std::plus{}
        );
        CHECK(out.size() == 2UZ);
        CHECK(sumID == 505);
    }

    SUBCASE("引き出す数以下の場合でもID重複は排除される") {
        for (auto [id, wage] : std::span{requests.begin(), 2}) nothing(market.request(id, wage));

        market.pickRequest(AgentID{101}, out, rng);

        CHECK(out.size() < 2UZ);
        CHECK(std::ranges::all_of(
            out | std::views::transform(&Request::firmID),
            [](const AgentID firmId) noexcept -> bool { return firmId != AgentID{101}; }
        ));
    }

    SUBCASE("clear呼び出しの場合、空が返る") {
        for (auto [id, wage] : requests) nothing(market.request(id, wage));
        market.clear();

        market.pickRequest(AgentID{42}, out, rng);

        CHECK(out.empty());
    }
}

TEST_CASE("Requestのテスト") {  // NOLINT
    constexpr auto entries = std::array{
        std::pair{AgentID{101}, 101}, std::pair{AgentID{202}, 202}, std::pair{AgentID{303}, 303}
    };
    auto  market  = Market{};
    auto& request = market.request(AgentID{42}, Wage{10.0});

    SUBCASE("デフォルトでentriesは空") { CHECK(request.entries().empty()); }

    SUBCASE("entriesが正しく返ることのテスト") {
        for (auto [id, power] : entries) nothing(request.entry(id, power));

        const auto entryIds = request.entries() | std::views::transform(&Entry::entrantId) |
                              std::views::transform(&AgentID::value);
        const auto sumIds = std::ranges::fold_left(entryIds, 0.0, std::plus{});
        CHECK(sumIds == 606);
    }
}
}  // namespace
}  // namespace abm::labor