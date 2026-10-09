#include "SimulationEngine.h"
#include <algorithm>
#include <chrono>
#include <iomanip>

```cpp
std::vector<int> SimulationEngine::makeSchedule(int T, int warmup, int budget) {
    std::vector<int> schedule;

    if (T < 0) T = 0;
    if (warmup < 0) warmup = 0;

    if (budget <= 0 || warmup >= T) {
        return schedule;
    }

    const long long duration = T - warmup;

    for (int i = 0; i < budget; ++i) {
        long long time = warmup + 1 + ((long long)i * duration) / budget;

        if (time > T) {
            break;
        }

        if (schedule.empty() || schedule.back() != static_cast<int>(time)) {
            schedule.push_back(static_cast<int>(time));
        }
    }

    return schedule;
}
```

    return s;
}

SimulationEngine::SimulationEngine(const SocialGraph& g, const SimulationConfig& cfg, InterventionStrategy* s)
    : graph_(g), cfg_(cfg), strategy_(s) {
    if (cfg_.ticks < 0) cfg_.ticks = 0;
    if (cfg_.warmup < 0) cfg_.warmup = 0;
    if (cfg_.budget < 0) cfg_.budget = 0;
    schedule_ = makeSchedule(cfg_.ticks, cfg_.warmup, cfg_.budget);
}

```cpp
SimulationResult SimulationEngine::run(int source, uint64_t runSeed, std::ostream* log) {
    auto startTime = std::chrono::steady_clock::now();

    SimulationResult result;
    CascadeModel model(graph_, cfg_.misProb, cfg_.factProb);
    ModeratorBudget moderator(cfg_.budget);

    const uint64_t strategySeed = runSeed + 1000000007ULL;

    if (strategy_ != nullptr) {
        strategy_->reset(strategySeed);
    }

    model.seedMisinformation(source);

    if (log != nullptr) {
        *log << "Tick 0: source = User " << source << " (initialised)\n";
    }
```


    size_t next = 0;
    for (int t = 1; t <= cfg_.ticks; t++) {
        TickReport rep = model.step(runSeed);
        r.terminationTick = t;
        if (log) *log << "Tick " << t << ": +" << rep.newMisinformed << " misinformed, +"
                      << rep.newCorrected << " corrected | reach=" << model.misinformationReach()
                      << " misinformed-now=" << model.currentMisinformed() << "\n";

        ```cpp
if (strategy_ && next < schedule_.size() && schedule_[next] == t) {
    int index = static_cast<int>(next);
    next++;

    if (budget.canIntervene()) {
        std::vector<int> candidates;

        for (int u = 0; u < graph_.numUsers(); ++u) {
            if (model.stateOf(u) != UserState::CORRECTED) {
                candidates.push_back(u);
            }
        }

        if (!candidates.empty()) {
            InterventionContext ctx{
                graph_, model, t, cfg_.ticks, index, strategySeed, candidates
            };

            int selected = strategy_->selectCandidate(ctx);

            if (selected >= 0 && budget.useIntervention() &&
                model.correctUser(selected)) {

                r.selectedUsers.push_back(selected);

                if (log) {
                    *log << "  >> INTERVENTION #" << (index + 1)
                         << " (" << strategy_->name() << "), "
                         << candidates.size() << " eligible candidates\n";

                    auto scores = strategy_->lastScores();

                    std::sort(scores.begin(), scores.end(),
                        [](const auto& a, const auto& b) {
                            return a.second != b.second
                                ? a.second > b.second
                                : a.first < b.first;
                        });

                    for (size_t i = 0; i < scores.size() && i < 5; ++i) {
                        *log << "       User " << scores[i].first
                             << " -> score " << std::fixed
                             << std::setprecision(2) << scores[i].second << "\n";
                    }

                    *log << "     Selected: User " << selected
                         << "  (budget left " << budget.getRb() << ")\n";
                }
            }
        }
    }
}
```

                    }
                } else if (log) *log << "  >> Intervention skipped: no eligible candidates\n";
            }
        }
    }

    r.misinformationReach = model.misinformationReach();
    r.finalMisinformed = model.currentMisinformed();
    r.factCheckReach = model.factCheckReach();
    r.directCorrections = model.directCorrections();
    r.propagatedCorrections = model.propagatedCorrections();
    r.interventionsUsed = (int)r.selectedUsers.size();
    r.runtimeMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    return r;
}
