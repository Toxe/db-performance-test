#include "statistics.h"

#include <algorithm>
#include <numeric>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

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

void show_stats(const std::string& url, const std::vector<float>& durations, const int num_errors)
{
    spdlog::get("combined")->info("{} --> successful: {}, errors: {}, mean: {:.2f}ms, median: {:.2f}ms",
        url, durations.size(), num_errors, mean(durations), median(durations));
}
