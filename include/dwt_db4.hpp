#pragma once
#include <vector>

// 1D DWT and inverse DWT
void dwt1D(const std::vector<float>& input, std::vector<float>& approx, std::vector<float>& detail);
std::vector<float> idwt1D(const std::vector<float>& approx, const std::vector<float>& detail);

// 2D DWT and inverse DWT
void dwt2D_db4(const std::vector<std::vector<float>>& input,
               std::vector<std::vector<float>>& LL,
               std::vector<std::vector<float>>& LH,
               std::vector<std::vector<float>>& HL,
               std::vector<std::vector<float>>& HH);

std::vector<std::vector<float>> idwt2D_db4(const std::vector<std::vector<float>>& LL,
                                           const std::vector<std::vector<float>>& LH,
                                           const std::vector<std::vector<float>>& HL,
                                           const std::vector<std::vector<float>>& HH);