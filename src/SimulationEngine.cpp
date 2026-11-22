#include "SimulationEngine.h"
#include <algorithm>
#include <chrono>
#include <iomanip>

SimulationEngine::SimulationEngine(const SocialGraph& g, const SimulationConfig& cfg, InterventionStrategy* s)
    : graph_(g), cfg_(cfg), strategy_(s) {
    if (cfg_.ticks < 0) cfg_.ticks = 0;
    if (cfg_.warmup < 0) cfg_.warmup = 0;
    if (cfg_.budget < 0) cfg_.budget = 0;
}

SimulationResult SimulationEngine::run(int source, uint64_t runSeed, std::ostream* log, RunTrace* trace) {
    auto t0 = std::chrono::steady_clock::now();
    SimulationResult r;
    CascadeModel model(graph_, cfg_.misProb, cfg_.factProb);
    ModeratorBudget budget(cfg_.budget);
    // The strategy gets its own seed, different from the real run's seed, so its "what if" trials
    // never reuse the random numbers of the real simulation.
    const uint64_t strategySeed = runSeed + 1000000007ULL;
    if (strategy_) strategy_->reset(strategySeed);
    bool seeded = model.seedMisinformation(source);
    if (trace) {
        trace->source = source;
        trace->ticks.assign(cfg_.ticks + 1, TickTrace());
        if (seeded) trace->ticks[0].misinformed.push_back({source, -1});
    }
    if (log) *log << "Tick 0: source = User " << source << " (initialised)\n";

    bool idleShown = false;
    // The simulation always runs exactly T ticks, even if nothing can spread any more.
    for (int t = 1; t <= cfg_.ticks; t++) {
        TickEvents events;
        TickReport rep = model.step(runSeed, trace ? &events : nullptr);
        if (trace) { trace->ticks[t].misinformed = events.misinformed; trace->ticks[t].corrected = events.corrected; }
        r.terminationTick = t;
        if (log) *log << "Tick " << t << ": +" << rep.newMisinformed << " misinformed, +"
                      << rep.newCorrected << " corrected | reach=" << model.misinformationReach()
                      << " misinformed-now=" << model.currentMisinformed() << "\n";

        // The moderator acts once per tick, starting right after the warm-up, until the budget is used up,
        // but only while a fact-check can still change something (one rule for every strategy):
        //   - at least one more tick must follow (a fact-check needs a later tick to spread; on the last tick it is useless), and
        //   - the rumour must still be alive: some misinformed user has not spread yet (otherwise the reach is already fixed).
        // Every strategy is offered exactly the users of eligibleUsers(). A strategy that stops the rumour sooner
        // therefore also stops using budget sooner.
        if (strategy_ && t > cfg_.warmup && budget.canIntervene()) {
            const bool ticksRemain = t < cfg_.ticks;
            if (ticksRemain && model.misinformationActive()) {
                int idx = (int)r.selectedUsers.size();            // 0-based number of this intervention
                std::vector<int> cand = model.eligibleUsers();     // never empty here: the frontier users are eligible
                InterventionContext ctx{graph_, model, t, cfg_.ticks, idx, strategySeed, cand};
                int pick = strategy_->selectCandidate(ctx);
                if (pick >= 0 && budget.useIntervention() && model.correctUser(pick)) {
                    r.selectedUsers.push_back(pick);
                    if (trace) {
                        InterventionTrace it; it.user = pick; it.candidates = (int)cand.size();
                        auto sc = strategy_->lastScores();
                        std::sort(sc.begin(), sc.end(), [](auto& a, auto& b) {
                            return a.second != b.second ? a.second > b.second : a.first < b.first; });
                        if (sc.size() > 5) sc.resize(5);
                        it.top = sc;
                        trace->ticks[t].interventions.push_back(it);
                    }
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
            } else {
                // The moderator waits: a fact-check now could no longer change the outcome. The budget is kept.
                if (trace) trace->ticks[t].skipped = true;
                if (log && !idleShown) {                           // said once, at the first idle tick
                    *log << "  >> Moderator idle: " << (ticksRemain ? "the rumour has stopped spreading, so no more fact-checks are needed"
                                                                    : "no tick left for a fact-check to spread") << "\n";
                    idleShown = true;
                }
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
