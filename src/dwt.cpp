#include "dwt.hpp"
#include <vector>
#include <cmath>
#include <iostream>

// db4 Low-pass decomposition filter
const std::vector<double> db4_Lo_D = {
    -0.010597401785069,  0.032883011666885,
     0.030841381835560, -0.187034811719093,
    -0.027983769416984,  0.630880767929590,
     0.714846570552542,  0.230377813308897
};

// db4 High-pass decomposition filter
const std::vector<double> db4_Hi_D = {
    -0.230377813308897,  0.714846570552542,
    -0.630880767929590, -0.027983769416984,
     0.187034811719093,  0.030841381835560,
    -0.032883011666885, -0.010597401785069
};

// db4 Reconstruction filters
const std::vector<double> db4_Lo_R = {
     0.230377813308897,  0.714846570552542,
     0.630880767929590, -0.027983769416984,
    -0.187034811719093,  0.030841381835560,
     0.032883011666885, -0.010597401785069
};

const std::vector<double> db4_Hi_R = {
    -0.010597401785069, -0.032883011666885,
     0.030841381835560,  0.187034811719093,
    -0.027983769416984, -0.630880767929590,
     0.714846570552542, -0.230377813308897
};

// Convolution + downsampling
std::vector<double> convolveDownsample(const std::vector<double>& signal, const std::vector<double>& filter) {
    int len = signal.size();
    int flen = filter.size();
    std::vector<double> result;
    for (int i = 0; i <= len - flen; i += 2) {
        double sum = 0.0;
        for (int j = 0; j < flen; ++j)
            sum += signal[i + j] * filter[j];
        result.push_back(sum);
    }
    return result;
}

// Convolution + upsampling (for IDWT)
std::vector<double> upsampleAndConvolve(const std::vector<double>& signal, const std::vector<double>& filter) {
    int up_len = signal.size() * 2;
    std::vector<double> upsampled(up_len, 0.0);
    for (size_t i = 0; i < signal.size(); ++i)
        upsampled[i * 2] = signal[i];

    int flen = filter.size();
    std::vector<double> result(up_len + flen - 1, 0.0);

    for (int i = 0; i < up_len; ++i)
        for (int j = 0; j < flen; ++j)
            if (i - j >= 0 && i - j < up_len)
                result[i] += upsampled[i - j] * filter[j];

    result.resize(up_len);
    return result;
}

// 2D Forward DWT using db4
void dwt2D(const std::vector<std::vector<float>>& input,
           std::vector<std::vector<float>>& LL,
           std::vector<std::vector<float>>& LH,
           std::vector<std::vector<float>>& HL,
           std::vector<std::vector<float>>& HH) {
    int rows = input.size();
    int cols = input[0].size();

    std::vector<std::vector<double>> temp_low(rows), temp_high(rows);
    for (int i = 0; i < rows; ++i) {
        std::vector<double> row(input[i].begin(), input[i].end());
        temp_low[i] = convolveDownsample(row, db4_Lo_D);
        temp_high[i] = convolveDownsample(row, db4_Hi_D);
    }

    int out_cols = temp_low[0].size();
    int out_rows = (rows - db4_Lo_D.size()) / 2 + 1;

    LL.resize(out_rows, std::vector<float>(out_cols));
    LH.resize(out_rows, std::vector<float>(out_cols));
    HL.resize(out_rows, std::vector<float>(out_cols));
    HH.resize(out_rows, std::vector<float>(out_cols));

    for (int j = 0; j < out_cols; ++j) {
        std::vector<double> colL, colH;
        for (int i = 0; i < rows; ++i) {
            if (j < temp_low[i].size()) colL.push_back(temp_low[i][j]);
            if (j < temp_high[i].size()) colH.push_back(temp_high[i][j]);
        }

        std::vector<double> LL_col = convolveDownsample(colL, db4_Lo_D);
        std::vector<double> LH_col = convolveDownsample(colL, db4_Hi_D);
        std::vector<double> HL_col = convolveDownsample(colH, db4_Lo_D);
        std::vector<double> HH_col = convolveDownsample(colH, db4_Hi_D);

        for (int i = 0; i < LL_col.size(); ++i) {
            LL[i][j] = static_cast<float>(LL_col[i]);
            LH[i][j] = static_cast<float>(LH_col[i]);
            HL[i][j] = static_cast<float>(HL_col[i]);
            HH[i][j] = static_cast<float>(HH_col[i]);
        }
    }
}

// 2D Inverse DWT using db4
std::vector<std::vector<float>> idwt2D(const std::vector<std::vector<float>>& LL,
                                       const std::vector<std::vector<float>>& LH,
                                       const std::vector<std::vector<float>>& HL,
                                       const std::vector<std::vector<float>>& HH) {
    int rows = LL.size();
    int cols = LL[0].size();

    std::vector<std::vector<double>> low(rows), high(rows);
    for (int i = 0; i < rows; ++i) {
        std::vector<double> colL(cols), colH(cols);
        for (int j = 0; j < cols; ++j) {
            colL[j] = LL[i][j] + LH[i][j];
            colH[j] = HL[i][j] + HH[i][j];
        }

        auto rec_low = upsampleAndConvolve(colL, db4_Lo_R);
        auto rec_high = upsampleAndConvolve(colH, db4_Hi_R);

        std::vector<double> merged(rec_low.size());
        for (size_t j = 0; j < merged.size(); ++j)
            merged[j] = rec_low[j] + rec_high[j];

        low[i] = merged;
    }

    std::vector<std::vector<float>> output(low[0].size(), std::vector<float>(low.size()));
    for (size_t j = 0; j < low[0].size(); ++j) {
        std::vector<double> col(low.size());
        for (size_t i = 0; i < low.size(); ++i)
            col[i] = low[i][j];

        auto rec_col = upsampleAndConvolve(col, db4_Lo_R);

        for (size_t i = 0; i < rec_col.size(); ++i) {
            if (i < output.size() && j < output[0].size())
                output[i][j] = static_cast<float>(rec_col[i]);
        }
    }

    return output;
}
