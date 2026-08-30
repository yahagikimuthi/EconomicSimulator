#pragma once

#include <filesystem>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "others/setting.hpp"
#include "values/date.hpp"
#include "world/common.hpp"

namespace abm {
class Logger final {
  public:
    [[nodiscard]] explicit Logger() noexcept
        : file_{[]() noexcept -> HighFive::File {
              namespace fs        = std::filesystem;
              const auto filepath = static_cast<std::string>(setting::simulationResultOutputPath);
              const auto path     = fs::path{filepath};
              if (path.has_parent_path()) fs::create_directories(path.parent_path());
              return HighFive::File{
                  filepath,
                  HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
              };
          }()} {}

    [[nodiscard]] auto isValid() const noexcept -> bool { return file_.isValid(); }

    void save(const CensusDropBox& dropBox, const Date date) noexcept {
        namespace name = setting::save_name;
        auto groupPath = std::string{"/step_" + std::to_string(date.toFlatTime())};
        auto group     = HighFive::Group{file_.createGroup(groupPath)};

        auto create =
            [&group](std::string_view dataName, const std::vector<double>& data) noexcept -> void {
            group.createDataSet(static_cast<std::string>(dataName), data);
        };

        create(name::firmAssets, dropBox.firmAssets);
        create(name::postedEmployments, dropBox.postedEmployments);
        create(name::postedWages, dropBox.postedWages);
        create(name::employments, dropBox.employments);
        create(name::sumWages, dropBox.sumWages);
        create(name::prices, dropBox.prices);
        create(name::supplies, dropBox.supplies);
        create(name::markups, dropBox.markups);
        create(name::inventories, dropBox.inventories);
        create(name::householdAssets, dropBox.hholdAssets);
        create(name::wages, dropBox.wages);
    }

  private:
    HighFive::File file_;
};
}  // namespace abm