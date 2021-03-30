#include "statistics.h"

#include <algorithm>
#include <numeric>

#include <fmt/core.h>

float mean(const std::vector<float>& values)
{
    if (values.empty())
        return 0.0f;

    return std::accumulate(values.begin(), values.end(), 0.0f) / static_cast<float>(values.size());
}

float median(const std::vector<float>& values)
{
    if (values.empty())
        return 0.0f;

    std::vector<float> sorted_values{values};
    std::sort(sorted_values.begin(), sorted_values.end());

    if (sorted_values.size() % 2)
        return sorted_values[(sorted_values.size() - 1) / 2];
    else
        return (sorted_values[sorted_values.size() / 2 - 1] + sorted_values[sorted_values.size() / 2]) / 2.0f;
}

void show_stats(const std::vector<float>& durations, const int num_errors)
{
    fmt::print("successful: {}, errors: {}\n", durations.size(), num_errors);
    fmt::print("mean: {:.2f}ms\n", mean(durations));
    fmt::print("median: {:.2f}ms\n", median(durations));
}