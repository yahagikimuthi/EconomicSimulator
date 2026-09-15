# Details

Date : 2026-08-06 16:22:42

Directory /workspaces/EconomicSimulator

Total : 48 files,  10060 codes, 565 comments, 2114 blanks, all 12739 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [CMakeLists.txt](/CMakeLists.txt) | CMake | 81 | 0 | 18 | 99 |
| [include/analysis/analysis.hpp](/include/analysis/analysis.hpp) | C++ | 31 | 0 | 7 | 38 |
| [include/analysis/context\_task.hpp](/include/analysis/context_task.hpp) | C++ | 65 | 0 | 19 | 84 |
| [include/analysis/data\_manager.hpp](/include/analysis/data_manager.hpp) | C++ | 99 | 0 | 10 | 109 |
| [include/analysis/pipeline.hpp](/include/analysis/pipeline.hpp) | C++ | 21 | 0 | 8 | 29 |
| [include/components/common.hpp](/include/components/common.hpp) | C++ | 40 | 0 | 9 | 49 |
| [include/components/goods\_demander.hpp](/include/components/goods_demander.hpp) | C++ | 40 | 0 | 6 | 46 |
| [include/components/goods\_supplier.hpp](/include/components/goods_supplier.hpp) | C++ | 139 | 0 | 13 | 152 |
| [include/components/labor\_demander/concepts.hpp](/include/components/labor_demander/concepts.hpp) | C++ | 51 | 0 | 6 | 57 |
| [include/components/labor\_demander/hr\_manager.hpp](/include/components/labor_demander/hr_manager.hpp) | C++ | 28 | 0 | 5 | 33 |
| [include/components/labor\_demander/labor\_demander.hpp](/include/components/labor_demander/labor_demander.hpp) | C++ | 47 | 0 | 4 | 51 |
| [include/components/labor\_demander/planner.hpp](/include/components/labor_demander/planner.hpp) | C++ | 52 | 0 | 9 | 61 |
| [include/components/labor\_demander/recruiter.hpp](/include/components/labor_demander/recruiter.hpp) | C++ | 41 | 0 | 11 | 52 |
| [include/components/labor\_supplier.hpp](/include/components/labor_supplier.hpp) | C++ | 183 | 0 | 19 | 202 |
| [include/config.hpp](/include/config.hpp) | C++ | 40 | 0 | 8 | 48 |
| [include/core/base.hpp](/include/core/base.hpp) | C++ | 40 | 0 | 9 | 49 |
| [include/core/engine.hpp](/include/core/engine.hpp) | C++ | 66 | 1 | 15 | 82 |
| [include/core/forward.hpp](/include/core/forward.hpp) | C++ | 28 | 0 | 2 | 30 |
| [include/core/values/common.hpp](/include/core/values/common.hpp) | C++ | 65 | 0 | 8 | 73 |
| [include/core/values/goods.hpp](/include/core/values/goods.hpp) | C++ | 105 | 0 | 7 | 112 |
| [include/core/values/labor.hpp](/include/core/values/labor.hpp) | C++ | 109 | 0 | 8 | 117 |
| [include/doctest.h](/include/doctest.h) | C++ | 6,833 | 541 | 1,603 | 8,977 |
| [include/helper.hpp](/include/helper.hpp) | C++ | 74 | 0 | 13 | 87 |
| [include/orchestrator/goods.hpp](/include/orchestrator/goods.hpp) | C++ | 42 | 0 | 9 | 51 |
| [include/orchestrator/labor.hpp](/include/orchestrator/labor.hpp) | C++ | 60 | 0 | 10 | 70 |
| [include/orchestrator/updates\_loggings.hpp](/include/orchestrator/updates_loggings.hpp) | C++ | 8 | 0 | 3 | 11 |
| [include/world/message.hpp](/include/world/message.hpp) | C++ | 101 | 0 | 26 | 127 |
| [scripts/load.py](/scripts/load.py) | Python | 36 | 0 | 6 | 42 |
| [scripts/main.py](/scripts/main.py) | Python | 14 | 0 | 5 | 19 |
| [scripts/plot.py](/scripts/plot.py) | Python | 50 | 0 | 12 | 62 |
| [src/analysis/pipeline.cpp](/src/analysis/pipeline.cpp) | C++ | 32 | 0 | 7 | 39 |
| [src/app\_main.cpp](/src/app_main.cpp) | C++ | 7 | 0 | 2 | 9 |
| [src/components/components.cpp](/src/components/components.cpp) | C++ | 41 | 0 | 4 | 45 |
| [src/components/goods\_demander.cpp](/src/components/goods_demander.cpp) | C++ | 54 | 0 | 8 | 62 |
| [src/components/goods\_supplier/calc\_employ.cpp](/src/components/goods_supplier/calc_employ.cpp) | C++ | 26 | 0 | 4 | 30 |
| [src/components/goods\_supplier/posting.cpp](/src/components/goods_supplier/posting.cpp) | C++ | 55 | 0 | 9 | 64 |
| [src/components/goods\_supplier/trade.cpp](/src/components/goods_supplier/trade.cpp) | C++ | 71 | 0 | 9 | 80 |
| [src/components/labor\_demander/hr\_manager.cpp](/src/components/labor_demander/hr_manager.cpp) | C++ | 48 | 0 | 6 | 54 |
| [src/components/labor\_demander/planner.cpp](/src/components/labor_demander/planner.cpp) | C++ | 67 | 0 | 9 | 76 |
| [src/components/labor\_demander/recruiter.inl](/src/components/labor_demander/recruiter.inl) | C++ | 82 | 0 | 10 | 92 |
| [src/engine/initialize.cpp](/src/engine/initialize.cpp) | C++ | 177 | 21 | 21 | 219 |
| [src/engine/run.cpp](/src/engine/run.cpp) | C++ | 108 | 2 | 24 | 134 |
| [src/orchestrator/updates\_loggings.cpp](/src/orchestrator/updates_loggings.cpp) | C++ | 42 | 0 | 8 | 50 |
| [tests/components/labor\_demander/test\_hr\_manager.cpp](/tests/components/labor_demander/test_hr_manager.cpp) | C++ | 189 | 0 | 36 | 225 |
| [tests/components/labor\_demander/test\_planner.cpp](/tests/components/labor_demander/test_planner.cpp) | C++ | 114 | 0 | 17 | 131 |
| [tests/components/labor\_demander/test\_recruiter.cpp](/tests/components/labor_demander/test_recruiter.cpp) | C++ | 336 | 0 | 46 | 382 |
| [tests/test\_helper.hpp](/tests/test_helper.hpp) | C++ | 20 | 0 | 6 | 26 |
| [tests/test\_main.cpp](/tests/test_main.cpp) | C++ | 2 | 0 | 0 | 2 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)