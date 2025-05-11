#pragma once
#include <vector>

// Forward 2D DWT using Daubechies db4
void dwt2D(const std::vector<std::vector<float>>& input,
           std::vector<std::vector<float>>& LL,
           std::vector<std::vector<float>>& LH,
           std::vector<std::vector<float>>& HL,
           std::vector<std::vector<float>>& HH);

// Inverse 2D DWT using Daubechies db4
std::vector<std::vector<float>> idwt2D(const std::vector<std::vector<float>>& LL,
                                       const std::vector<std::vector<float>>& LH,
                                       const std::vector<std::vector<float>>& HL,
                                       const std::vector<std::vector<float>>& HH);
