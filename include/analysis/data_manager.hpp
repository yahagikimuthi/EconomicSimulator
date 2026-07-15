#pragma once

#include <algorithm>
#include <filesystem>
#include <highfive/H5DataType.hpp>
#include <highfive/H5File.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "config.hpp"

namespace analysis {
class [[nodiscard]] InputDataManager {
  public:
    InputDataManager()
        : inFile_{config::setting::simulationResultOutputPath, HighFive::File::ReadOnly} {
        if (not inFile_.isValid()) {
            std::cerr << "Failed to load the file\n"
                      << "Path: "
                      << std::filesystem::absolute(config::setting::simulationResultOutputPath)
                      << '\n';
            std::abort();
        }
        stepKeys_ = inFile_.listObjectNames();
    }

    auto getStepKeys() const -> std::vector<std::string> {
        auto stepKeys{inFile_.listObjectNames()};
        std::ranges::sort(stepKeys, std::ranges::less{}, [](const std::string& step) -> int {
            return std::stoi(step.substr(5));
        });
        return stepKeys;
    }

    void read(
        const std::string& path, const std::string& dataName, std::vector<double>& container
    ) const {
        const auto group{getGroup(path)};
        if (not group) {
            container.clear();
            return;
        }
        if (not group->exist(dataName)) {
            std::cerr << "data: " << dataName << "does not exit\n";
            container.clear();
            return;
        }
        group->getDataSet(dataName).read(container);
    }

  private:
    auto getGroup(const std::string& path) const -> std::optional<HighFive::Group> {
        if (inFile_.exist(path)) {
            return inFile_.getGroup(path);
        }
        std::cerr << "group: " << path << "does not exist\n";
        return std::nullopt;
    }

    HighFive::File           inFile_;
    std::vector<std::string> stepKeys_;
};

class [[nodiscard]] OutputDataManager {
  public:
    OutputDataManager()
        : outFile_{
              config::setting::metricDataOutputPath,
              HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate
          } {
        if (not outFile_.isValid()) {
            std::cerr << "Failed to create the file\n"
                      << "Path: "
                      << std::filesystem::absolute(config::setting::metricDataOutputPath) << '\n';
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
}  // namespace analysis