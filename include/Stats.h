#ifndef STATS_H
#define STATS_H
#include <cmath>
#include <vector>

// Small statistics helpers used by the benchmark (header-only so the tests can use them too).
inline double mean(const std::vector<double>& v) {
    double sum = 0;
    for (double x : v) sum += x;
    return v.empty() ? 0 : sum / v.size();
}
inline double stddev(const std::vector<double>& v) {           // sample standard deviation
    if (v.size() < 2) return 0;
    double m = mean(v), sum = 0;
    for (double x : v) sum += (x - m) * (x - m);
    return std::sqrt(sum / (v.size() - 1));
}
// Two-sided 95% critical value of Student's t for `df` degrees of freedom
// (table for df <= 30, then a coarse step-down towards the normal value 1.96).
inline double tCritical95(int df) {
    static const double t[31] = {0, 12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228,
                                 2.201, 2.179, 2.160, 2.145, 2.131, 2.120, 2.110, 2.101, 2.093, 2.086,
                                 2.080, 2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042};
    if (df < 1) return 0;
    if (df <= 30) return t[df];
    if (df <= 60) return 2.00;
    if (df <= 120) return 1.98;
    return 1.96;
}
// Half-width of the 95% confidence interval of the mean: mean +/- ci95HalfWidth(v).
inline double ci95HalfWidth(const std::vector<double>& v) {
    if (v.size() < 2) return 0;
    return tCritical95((int)v.size() - 1) * stddev(v) / std::sqrt((double)v.size());
}
#endif