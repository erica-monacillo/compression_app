#pragma once
#include <vector>

void dwt2D(const std::vector<std::vector<float>>& input,
           std::vector<std::vector<float>>& LL,
           std::vector<std::vector<float>>& LH,
           std::vector<std::vector<float>>& HL,
           std::vector<std::vector<float>>& HH);

std::vector<std::vector<float>> idwt2D(const std::vector<std::vector<float>>& LL,
                                       const std::vector<std::vector<float>>& LH,
                                       const std::vector<std::vector<float>>& HL,
                                       const std::vector<std::vector<float>>& HH);
