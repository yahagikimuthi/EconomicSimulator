#pragma once

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <highfive/H5DataType.hpp>
#include <highfive/H5File.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "setting.hpp"

namespace abm::analysis {
class InputDataManager final {
  public:
    [[nodiscard]] InputDataManager()
        : inFile_{[]() -> HighFive::File {
              const auto filepath = static_cast<std::string>(setting::simulationResultOutputPath);

              if (const auto path = std::filesystem::path{filepath}; path.has_parent_path())
                  std::filesystem::create_directories(path.parent_path());
              return HighFive::File{filepath, HighFive::File::ReadOnly};
          }()} {
        if (not inFile_.isValid()) {
            std::cerr << "Failed to load the file\n"
                      << "Path: "
                      << std::filesystem::absolute(
                             static_cast<std::string>(setting::simulationResultOutputPath)
                         )
                      << '\n';
            std::abort();
        }
    }

    [[nodiscard]] auto getStepKeys() const -> std::vector<std::string> {
        auto stepKeys = inFile_.listObjectNames();
        std::ranges::sort(stepKeys, std::ranges::less{}, [](const std::string& step) -> int {
            return std::stoi(step.substr(5));  // NOLINT
        });
        return stepKeys;
    }

    void read(const std::string& path, std::string_view dataName, std::vector<double>& container)
        const {
        const auto group = getGroup(path);
        if (not group) {
            container.clear();
            return;
        }
        if (not group->exist(static_cast<std::string>(dataName))) {
            std::cerr << "data: " << static_cast<std::string>(dataName) << "does not exit\n";
            container.clear();
            return;
        }
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
    [[nodiscard]] OutputDataManager()
        : outFile_{[]() -> HighFive::File {
              const auto filepath = static_cast<std::string>(setting::metricDataOutputPath);
              if (const auto path = std::filesystem::path{filepath}; path.has_parent_path()) {
                  std::filesystem::create_directories(path.parent_path());
              }
              return HighFive::File{
                  filepath,
                  HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
              };
          }()} {
        if (not outFile_.isValid()) {
            std::cerr << "Failed to create the file\n"
                      << "Path: "
                      << std::filesystem::absolute(
                             static_cast<std::string>(setting::metricDataOutputPath)
                         )
                      << '\n';
            std::abort();
        }
    }

    void write(const std::string&& dataSetName, const std::vector<double>&& container) {
        outFile_.createDataSet<double>(dataSetName, HighFive::DataSpace::From(container))
            .write(container);
    }

  private:
    HighFive::File outFile_;
};
}  // namespace abm::analysis