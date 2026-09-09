#pragma once

#include <filesystem>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <string>
#include <string_view>

#include "others/setting.hpp"
#include "world/drop_box.hpp"

namespace abm {
class Logger final {
  public:
    [[nodiscard]] explicit Logger()
        : file_{[]() noexcept -> HighFive::File {
              namespace fs = std::filesystem;
              const auto filepath =
                  static_cast<std::string>(global_setting::simulationResultOutputPath);
              const auto path = fs::path{filepath};
              if (path.has_parent_path()) fs::create_directories(path.parent_path());
              return HighFive::File{
                  filepath,
                  HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
              };
          }()} {
        if (not isValid()) throw std::invalid_argument("cannot open file");
    }

    [[nodiscard]] auto isValid() const noexcept -> bool { return file_.isValid(); }

    void save(const CensusDropBox& dropBox, const int month) noexcept {
        namespace name = global_setting::save_name;
        auto groupPath = std::string{"/step_" + std::to_string(month)};
        auto group     = HighFive::Group{file_.createGroup(groupPath)};

        auto create =
            [&group](std::string_view dataName, const drop_box::Vec& data) noexcept -> void {
            group.createDataSet(static_cast<std::string>(dataName), data.get());
        };

        create(name::firmAssets, dropBox.finance.firmAssets);
        create(name::postedEmployments, dropBox.labor.postedEmployments);
        create(name::postedWages, dropBox.labor.postedWages);
        create(name::employments, dropBox.labor.employments);
        create(name::sumWages, dropBox.labor.wages);
        create(name::prices, dropBox.capital.prices);
        create(name::supplies, dropBox.capital.supplies);
        create(name::markups, dropBox.capital.markups);
        create(name::inventories, dropBox.capital.inventories);
        create(name::householdAssets, dropBox.finance.hholdAssets);
        create(name::wages, dropBox.labor.wages);
    }

  private:
    HighFive::File file_;
};
}  // namespace abm