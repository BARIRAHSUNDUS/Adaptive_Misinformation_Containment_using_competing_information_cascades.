#include "SimulationEngine.h"
#include <algorithm>
#include <chrono>
#include <iomanip>

std::vector<int> SimulationEngine::makeSchedule(int T, int warmup, int budget) {
    std::vector<int> s;
    if (T < 0) T = 0;
    if (warmup < 0) warmup = 0;
    if (budget <= 0 || warmup >= T) return s;
    const long long span = T - warmup;
    for (int i = 1; i <= budget; i++) {
        long long t = warmup + 1 + ((long long)(i - 1) * span) / budget;
        if (t > T) break;
        if (s.empty() || s.back() != (int)t) s.push_back((int)t);
    }
    return s;
}

SimulationEngine::SimulationEngine(const SocialGraph& g, const SimulationConfig& cfg, InterventionStrategy* s)
    : graph_(g), cfg_(cfg), strategy_(s) {
    if (cfg_.ticks < 0) cfg_.ticks = 0;
    if (cfg_.warmup < 0) cfg_.warmup = 0;
    if (cfg_.budget < 0) cfg_.budget = 0;
    schedule_ = makeSchedule(cfg_.ticks, cfg_.warmup, cfg_.budget);
}

SimulationResult SimulationEngine::run(int source, uint64_t runSeed, std::ostream* log) {
    auto t0 = std::chrono::steady_clock::now();
    SimulationResult r;
    CascadeModel model(graph_, cfg_.misProb, cfg_.factProb);
    ModeratorBudget budget(cfg_.budget);
    const uint64_t strategySeed = runSeed + 1000000007ULL;
    if (strategy_) strategy_->reset(strategySeed);
    model.seedMisinformation(source);
    if (log) *log << "Tick 0: source = User " << source << " (initialised)\n";

    size_t next = 0;
    for (int t = 1; t <= cfg_.ticks; t++) {
        TickReport rep = model.step(runSeed);
        r.terminationTick = t;
        if (log) *log << "Tick " << t << ": +" << rep.newMisinformed << " misinformed, +"
                      << rep.newCorrected << " corrected | reach=" << model.misinformationReach()
                      << " misinformed-now=" << model.currentMisinformed() << "\n";

        if (strategy_ && next < schedule_.size() && schedule_[next] == t) {
            int idx = (int)next++;
            if (budget.canIntervene()) {
                std::vector<int> cand;
                for (int u = 0; u < graph_.numUsers(); u++)
                    if (model.stateOf(u) != UserState::CORRECTED) cand.push_back(u);
                if (!cand.empty()) {
                    InterventionContext ctx{graph_, model, t, cfg_.ticks, idx, strategySeed, cand};
                    int pick = strategy_->selectCandidate(ctx);
                    if (pick >= 0 && budget.useIntervention() && model.correctUser(pick)) {
                        r.selectedUsers.push_back(pick);
                        if (log) {
                            *log << "  >> INTERVENTION #" << (idx + 1) << " (" << strategy_->name()
                                 << "), " << cand.size() << " eligible candidates\n";
                            auto sc = strategy_->lastScores();
                            std::sort(sc.begin(), sc.end(), [](auto& a, auto& b) {
                                return a.second != b.second ? a.second > b.second : a.first < b.first; });
                            for (size_t i = 0; i < sc.size() && i < 5; i++)
                                *log << "       User " << sc[i].first << " -> score "
                                     << std::fixed << std::setprecision(2) << sc[i].second << "\n";
                            *log << "     Selected: User " << pick << "  (budget left "
                                 << budget.getRb() << ")\n";
                        }
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
