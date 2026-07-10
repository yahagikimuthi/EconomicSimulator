#include "strategies/labor_demander.hpp"

#include "world/message.hpp"

namespace labor_demander {
[[nodiscard]] auto registerMember(RegisterMemberView view) -> double {
    const auto myRequest = view.myRequest();

    double sumProductPower{};
    int    employeeCnt{};
    for (const auto entryRef : view.offerApplicants()) {
        auto& entry = entryRef.get();
        if (not entry.isAccept_) continue;
        sumProductPower += entry.productPower_;
        ++employeeCnt;
    }

    //! logをここで書き換えるのはまずい
    view.setLog(myRequest.wage_, employeeCnt);
    return sumProductPower;
}
}  // namespace labor_demander