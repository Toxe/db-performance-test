#pragma once

#include <vector>

float mean(const std::vector<float>& values);
float median(const std::vector<float>& values);
void show_stats(const std::vector<float>& durations, const int num_errors);
