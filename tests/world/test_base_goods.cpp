#include "world/base_goods.hpp"

#include "doctest.h"
#include "others/util.hpp"
#include "tests/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/common.hpp"

namespace abm::base_goods {
namespace {
TEST_CASE("Workspaceのテスト") {  // NOLINT
    auto space = Workspace{};

    SUBCASE("pickUpInputを呼び出した場合、inputが0になっていること") {
        const auto input = 10.0;
        space.addInput(input);

        [[maybe_unused]] const auto firstPicked  = space.takeout();
        const auto                  secondPicked = space.takeout();

        CHECK(secondPicked.isZero());
    }

    SUBCASE("addInputの入力値とtakeoutの出力が同じであること") {
        const auto input = 10.0;
        space.addInput(input);

        CHECK(space.takeout().value() == doctest::Approx(input));
    }

    SUBCASE("コピーコンストラクタが正しく作動すること") {
        const auto input = 10.0;
        space.addInput(input);

        auto otherSpace = space;
        CHECK(otherSpace.takeout().value() == doctest::Approx(input));
    }

    SUBCASE("コピー代入演算子が正しく作動すること") {
        const auto input      = 10.0;
        auto       otherSpace = space;
        otherSpace.addInput(input);

        space = otherSpace;

        CHECK(space.takeout().value() == doctest::Approx(input));
    }
}

TEST_CASE("Requestのテスト") {  // NOLINT
    auto           market        = Market<EMarket::Goods>{Date{1}};
    constexpr auto price         = Price{10.0};
    constexpr auto supply        = GoodsQuantity{100.0};
    constexpr auto payment       = Money{100.0};
    constexpr auto desiredAmount = payment / price;
    auto           entry         = Entry<EMarket::Goods>{AgentID{42}, price, supply, market};
    auto           request       = Request<EMarket::Goods>{payment, entry};

    SUBCASE("tradeAmountはデフォルトで0を返す") {
        CHECK(request.tradeAmount().value() == doctest::Approx(0.0));
    }

    SUBCASE("takeoutRemainPaidはデフォルトでpaymentを返す") {
        CHECK(request.takeoutRemainPaid().value() == doctest::Approx(payment.value()));
    }

    SUBCASE("takeoutRemainPaidは2回目で0を返す") {
        [[maybe_unused]] const auto first = request.takeoutRemainPaid();

        CHECK(request.takeoutRemainPaid().isZero());
    }

    SUBCASE("需要量分、tradeを行った場合、売上及び取引量が正しく計算され、支払い残額が0であること"
    ) {
        const auto sales = request.trade(desiredAmount);

        CHECK(sales.value() == doctest::Approx((price * desiredAmount).value()));
        CHECK(request.tradeAmount().value() == doctest::Approx(desiredAmount.value()));
        CHECK(request.takeoutRemainPaid().isZero());
    }

    SUBCASE(
        "需要量以下、tradeを行った場合、基本的には満額と同じだが、支払い残額が正であり正しく計算さ"
        "れていること"
    ) {
        const auto tradeAmount = desiredAmount / 2.0;
        const auto sales       = request.trade(tradeAmount);

        CHECK(sales.value() == doctest::Approx((price * tradeAmount).value()));
        CHECK(request.tradeAmount().value() == doctest::Approx(tradeAmount.value()));
        const auto remain = request.takeoutRemainPaid();
        CHECK(remain.value() == doctest::Approx((payment - sales).value()));
        CHECK(remain.isPositive());
    }
}

TEST_CASE("Entryのテスト") {  // NOLINT
    constexpr auto date   = Date{1};
    constexpr auto id     = AgentID{42};
    constexpr auto price  = Price{10.0};
    constexpr auto supply = GoodsQuantity{100.0};
    auto           market = Market<EMarket::Goods>{date};
    auto           entry  = Entry<EMarket::Goods>{id, price, supply, market};

    SUBCASE("requestsはデフォルトで空") { CHECK(entry.requests().empty()); }

    SUBCASE("requestsが正しく返ること") {
        nothing(entry.request(Money{101.0}));
        nothing(entry.request(Money{102.0}));
        nothing(entry.request(Money{103.0}));

        const auto requests = entry.requests();
        CHECK(requests.size() == 3UZ);
        CHECK(requests[0].payment == Money{101.0});
        CHECK(requests[1].payment == Money{102.0});
        CHECK(requests[2].payment == Money{103.0});
    }

    SUBCASE("デフォルトで有効") { CHECK(entry.isValid()); }

    SUBCASE("disableを行った場合、無効になること") {
        entry.disable();
        CHECK(not entry.isValid());
    }
}

TEST_CASE("Marketのテスト") {  // NOLINT
    auto date   = Date{1};
    auto market = Market<EMarket::Goods>{date};

    SUBCASE("pickしてもデフォルトはnull") {
        auto       rng  = RandomGenerator{{}};
        const auto pick = market.pickEntry(AgentID{42}, 100, rng);
        CHECK(not pick.has_value());
    }

    SUBCASE("disableしたentryについて2日目以降は再利用を行う") {
        auto&       entry = market.entry(AgentID{42}, Price{10.0}, GoodsQuantity{100.0});
        auto* const ptr   = &entry;

        entry.disable();
        ++ ++date;

        auto& newEntry = market.entry(AgentID{-1}, Price{1.0}, GoodsQuantity{1.0});

        CHECK(ptr == &newEntry);
    }

    SUBCASE("disableしたentryについて2日目より前は再利用を行わない") {
        constexpr auto id     = AgentID{-1};
        constexpr auto price  = Price{1.0};
        constexpr auto supply = GoodsQuantity{1.0};

        auto&             entry = market.entry(id, price, supply);
        const auto* const ptr   = &entry;
        entry.disable();

        const auto& first = market.entry(id, price, supply);
        CHECK(ptr != &first);

        const auto& second = market.entry(id, price, supply);
        CHECK(ptr != &second);

        ++date;

        const auto& third = market.entry(id, price, supply);
        CHECK(ptr != &third);
    }

    SUBCASE("pickEntryが有効なentryをpickするかのテスト") {
        auto& entry  = market.entry(AgentID{101}, Price{101}, GoodsQuantity{101});
        auto  rng    = makeRng();
        auto  picked = market.pickEntry(AgentID{-1}, 1, rng);

        CHECK(picked);
        CHECK(picked->isValid());
        CHECK(&*picked == &entry);
    }
}
}  // namespace
}  // namespace abm::base_goods