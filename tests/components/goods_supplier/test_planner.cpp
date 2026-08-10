#include "components/goods_supplier.hpp"

#include "core/values/goods.hpp"
#include "doctest.h"
#include "tests/test_helper.hpp"
#include "world/message.hpp"

using test::equal;

namespace goods::supplier {
TEST_CASE("calcDesiredEmployのテスト") {  // NOLINT
    struct Input {
        GoodsQuantity inventory;
        double        firmProductPower;
        GoodsQuantity targetSupply;
        GoodsQuantity lastSupply;
        HeadCount     employeeCnt;
        double        laborSupplierCnt;
    };
    auto makeProducer{[](const Input& input) -> Producer {
        return {world::Workspace{}, input.firmProductPower, input.inventory};
    }};

    SUBCASE("正常系") {
        Input input{
            .inventory        = GoodsQuantity{10.0},
            .firmProductPower = 5.0,
            .targetSupply     = GoodsQuantity{100.0},
            .lastSupply       = GoodsQuantity{50.0},
            .employeeCnt      = HeadCount{5.0},
            .laborSupplierCnt = 20.0
        };
        auto      producer{makeProducer(input)};
        HeadCount employ{producer.calcDesiredEmploy(
            input.targetSupply, input.lastSupply, input.employeeCnt, input.laborSupplierCnt
        )};
        CHECK(equal(employ, HeadCount{9.0}));
    }

    SUBCASE("切り上げの検証") {
        Input input{
            .inventory        = GoodsQuantity{0.0},
            .firmProductPower = 1.0,
            .targetSupply     = GoodsQuantity{100.0},
            .lastSupply       = GoodsQuantity{30.0},
            .employeeCnt      = HeadCount{10.0},
            .laborSupplierCnt = 50.0
        };
        auto      producer{makeProducer(input)};
        HeadCount employ{producer.calcDesiredEmploy(
            input.targetSupply, input.lastSupply, input.employeeCnt, input.laborSupplierCnt
        )};
        CHECK(equal(employ, HeadCount{34.0}));
    }

    SUBCASE("従業員数が0の場合") {
        Input input{
            .inventory        = GoodsQuantity{0.0},
            .firmProductPower = 1.0,
            .targetSupply     = GoodsQuantity{50.0},
            .lastSupply       = GoodsQuantity{0.0},
            .employeeCnt      = HeadCount{5.0},
            .laborSupplierCnt = 10.0
        };
        auto      producer{makeProducer(input)};
        HeadCount employ{producer.calcDesiredEmploy(
            input.targetSupply, input.lastSupply, input.employeeCnt, input.laborSupplierCnt
        )};
        CHECK(equal(employ, HeadCount{1.0}));
    }

    SUBCASE("労働供給上限によるキャップ") {
        Input input{
            .inventory        = GoodsQuantity{0.0},
            .firmProductPower = 1.0,
            .targetSupply     = GoodsQuantity{1000.0},
            .lastSupply       = GoodsQuantity{10.0},
            .employeeCnt      = HeadCount{1.0},
            .laborSupplierCnt = 10.0
        };
        auto      producer{makeProducer(input)};
        HeadCount employ{producer.calcDesiredEmploy(
            input.targetSupply, input.lastSupply, input.employeeCnt, input.laborSupplierCnt
        )};
        CHECK(equal(employ, HeadCount{10.0}));
    }

    SUBCASE("在庫が目標供給量を上回っている場合") {
        Input input{
            .inventory        = GoodsQuantity{150.0},
            .firmProductPower = 1.0,
            .targetSupply     = GoodsQuantity{100.0},
            .lastSupply       = GoodsQuantity{50.0},
            .employeeCnt      = HeadCount{5.0},
            .laborSupplierCnt = 10.0
        };
        auto      producer{makeProducer(input)};
        HeadCount employ{producer.calcDesiredEmploy(
            input.targetSupply, input.lastSupply, input.employeeCnt, input.laborSupplierCnt
        )};
        CHECK(equal(employ, HeadCount{-5.0}));
    }
}
}  // namespace goods::supplier