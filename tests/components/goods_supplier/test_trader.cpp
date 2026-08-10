#include "components/goods_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <pcg_random.hpp>

#include "core/values/goods.hpp"
#include "doctest.h"
#include "tests/components/goods_supplier/test_helper.hpp"
#include "tests/test_helper.hpp"
#include "world/message.hpp"

using test::equal;

namespace goods::supplier {

TEST_CASE("Trader::postのテスト") {  // NOLINT
    struct Input {
        GoodsQuantity                             supply;
        Price                                     pricePlan;
        tbb::concurrent_vector<world::GoodsEntry> entryBox;
    };

    struct Expect {
        bool               isPosting;
        std::size_t        entryBoxSize;
        GoodsQuantity      inventory;
        bool               myEntryHasValue;
        world::GoodsEntry* myEntryPtr;
    };

    auto makeTrader{[]() -> Trader {
        pcg32 rng{test::makeSeed(), test::makeSeed()};
        return Trader{rng};
    }};

    SUBCASE("供給量が0の場合") {
        Input input{
            .supply    = GoodsQuantity{0.0},
            .pricePlan = Price{100.0},
            .entryBox  = {},
        };
        const Expect expect{
            .isPosting       = false,
            .entryBoxSize    = 0,
            .inventory       = GoodsQuantity{0.0},
            .myEntryHasValue = false,
            .myEntryPtr      = nullptr,
        };

        auto trader = makeTrader();
        trader.post(input.supply, input.pricePlan, input.entryBox);

        TraderTester tester{trader};
        CHECK(tester.isPosting() == expect.isPosting);
        CHECK(input.entryBox.size() == expect.entryBoxSize);
        CHECK(equal(tester.inventory(), expect.inventory));
        CHECK(tester.myEntry().has_value() == expect.myEntryHasValue);
    }

    SUBCASE("供給量が0より大きい場合") {
        Input input{
            .supply    = GoodsQuantity{10.0},
            .pricePlan = Price{150.0},
            .entryBox  = {},
        };

        auto trader = makeTrader();
        trader.post(input.supply, input.pricePlan, input.entryBox);

        const Expect expect{
            .isPosting       = true,
            .entryBoxSize    = 1,
            .inventory       = input.supply,
            .myEntryHasValue = true,
            .myEntryPtr      = &input.entryBox.back(),
        };

        TraderTester tester{trader};
        CHECK(tester.isPosting() == expect.isPosting);
        CHECK(input.entryBox.size() == expect.entryBoxSize);
        CHECK(equal(tester.inventory(), expect.inventory));
        CHECK(tester.myEntry().has_value() == expect.myEntryHasValue);
        CHECK(&*tester.myEntry() == expect.myEntryPtr);

        const auto& entry = input.entryBox.back();
        CHECK(equal(entry.price, input.pricePlan));
        CHECK(equal(entry.supply, input.supply));
    }
}
}  // namespace goods::supplier