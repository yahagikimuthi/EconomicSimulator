#pragma once

#include <algorithm>
#include <filesystem>
#include <highfive/H5DataType.hpp>
#include <highfive/H5File.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "others/setting.hpp"

namespace abm::analysis {
class InputDataManager final {
  public:
    explicit InputDataManager()
        : inFile_{[]() noexcept -> HighFive::File {
              namespace fs        = std::filesystem;
              const auto filepath = static_cast<std::string>(setting::simulationResultOutputPath);
              const auto path     = fs::path{filepath};
              path.has_parent_path();
              if (path.has_parent_path()) fs::create_directories(path.parent_path());
              return HighFive::File{filepath, HighFive::File::ReadOnly};
          }()} {}

    [[nodiscard]] auto getStepKeys() const -> std::vector<std::string> {
        auto stepKeys = inFile_.listObjectNames();
        std::ranges::sort(
            stepKeys,
            std::ranges::less{},
            [](const std::string& step) noexcept -> int {
                return std::stoi(step.substr(5));  // NOLINT
            }
        );
        return stepKeys;
    }

    void read(const std::string& path, std::string_view dataName, std::vector<double>& container)
        const {
        const auto group = getGroup(path);
        if (not group) {
            container.clear();
        } else if (not group->exist(static_cast<std::string>(dataName))) {
            std::cerr << "data: " << static_cast<std::string>(dataName) << "does not exit\n";
            container.clear();

        } else
            group->getDataSet(static_cast<std::string>(dataName)).read(container);
    }

  private:
    [[nodiscard]] auto getGroup(const std::string& path) const -> std::optional<HighFive::Group> {
        if (inFile_.exist(path)) {
            return inFile_.getGroup(path);
        }
        std::cerr << "group: " << path << "does not exist\n";
        return std::nullopt;
    }

    HighFive::File inFile_;
};

class OutputDataManager final {
  public:
    explicit OutputDataManager()
        : outFile_{[]() noexcept -> HighFive::File {
              namespace fs        = std::filesystem;
              const auto filepath = static_cast<std::string>(setting::metricDataOutputPath);
              const auto path     = fs::path{filepath};
              if (path.has_parent_path()) {
                  fs::create_directories(path.parent_path());
              }
              return HighFive::File{
                  filepath,
                  HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
              };
          }()} {}

    void write(const std::string&& dataSetName, const std::vector<double>&& container) {
        outFile_.createDataSet<double>(dataSetName, HighFive::DataSpace::From(container))
            .write(container);
    }

  private:
    HighFive::File outFile_;
};
}  // namespace abm::analysis