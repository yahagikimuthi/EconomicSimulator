#include "world/deposit.hpp"

#include "doctest.h"
#include "values/common.hpp"

namespace abm {
namespace {
TEST_CASE("BankAccountのテスト") {  // NOLINT
    auto account = BankAccount{AgentID{42}};

    SUBCASE("デフォルトで残高は0、出金しても戻り値は0") {
        CHECK(account.balance().isZero());
        CHECK(account.withdraw(Money{100}).isZero());
        CHECK(account.balance().isZero());
    }

    SUBCASE("預金した場合、残高がその分増える") {
        account.deposit(Money{10});
        CHECK(account.balance().value() == 10);
    }

    SUBCASE("預金する") {
        account.deposit(Money{100});

        SUBCASE("全額出金した場合、戻り値が満額で残高は0") {
            const auto withdraw = account.withdraw(Money{100});

            CHECK(withdraw.value() == 100);
            CHECK(account.balance().isZero());
        }

        SUBCASE("一部出金した場合、その分が戻り、残高が減額") {
            const auto withdraw = account.withdraw(Money{40});

            CHECK(withdraw.value() == 40);
            CHECK(account.balance().value() == 60);
        }

        SUBCASE("すべて出金した場合、残高が返り、残高は0") {
            const auto withdraw = account.withdraw(Money{200});

            CHECK(withdraw.value() == 100);
            CHECK(account.balance().isZero());
        }
    }
}
}  // namespace
}  // namespace abm