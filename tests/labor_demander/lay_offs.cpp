#include "strategies/labor_demander/lay_offs.hpp"
#include "doctest.h"

#include "world/message.hpp"

using namespace labor_demander;

TEST_CASE("解雇テスト") {
    world::CompanyBoard       board{0};
    labor_demander::Component comp{0, 0, &board};

    layoffs(LayOffsView{comp}, 4);
}