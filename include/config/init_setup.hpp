#pragma once

namespace Config::InitSetup::Setting {
constexpr unsigned int seed{42};
}  // namespace Config::InitSetup::Setting

namespace Config::Setting {
constexpr int seed{42};
}
namespace Config::LaborDemander {
constexpr double epsilonWage{1e-16};
}
namespace Config::GoodsSupplier {}

namespace Config::InitSetup::Firm {
constexpr int    count{100};
constexpr double minAsset{1000};
constexpr double maxAsset{10000};
constexpr double minProductivity{0.01};
constexpr double maxProductivity{0.1};
constexpr double epsilonWage{1e-16};
constexpr double epsilonMarkup{0.01};
constexpr int    minEmploy{1};
constexpr double demandShockMin{0.95};
constexpr double demandShockMax{1.05};
}  // namespace Config::InitSetup::Firm

namespace Config::InitSetup::HHold {
constexpr int    count{10000};
constexpr double minAsset{100};
constexpr double maxAsset{1000};
constexpr int    jobSampleCount{10};
constexpr int    jobEntryCount{10};
constexpr int    maxPurchaseFrequency{7};
}  // namespace Config::InitSetup::HHold