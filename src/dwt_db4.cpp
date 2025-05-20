#include "dwt_db4.hpp"
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

// Daubechies-4 coefficients
static const float h[] = { 0.4829629131f, 0.8365163037f, 0.2241438680f, -0.1294095226f };
static const float g[] = { -0.1294095226f, -0.2241438680f, 0.8365163037f, -0.4829629131f };

// Symmetric padding for 1D vector
static std::vector<float> padSymmetric(const std::vector<float>& input, int pad) {
    int N = input.size();
    std::vector<float> padded(N + 2 * pad);
    for (int i = 0; i < pad; ++i) {
        padded[i] = input[pad - i];                      // mirror left
        padded[N + pad + i] = input[N - 2 - i];          // mirror right
    }
    for (int i = 0; i < N; ++i)
        padded[pad + i] = input[i];
    return padded;
}

// 1D DWT
void dwt1D(const std::vector<float>& input, std::vector<float>& approx, std::vector<float>& detail) {
    int N = input.size();
    if (N < 4 || N % 2 != 0) {
        approx.clear();
        detail.clear();
        return;
    }
    std::vector<float> padded = padSymmetric(input, 2);
    approx.resize(N / 2);
    detail.resize(N / 2);
    for (int i = 0, k = 0; i < N; i += 2, ++k) {
        approx[k] = detail[k] = 0.0f;
        for (int j = 0; j < 4; ++j) {
            approx[k] += h[j] * padded[i + j];
            detail[k] += g[j] * padded[i + j];
        }
    }
}

// 1D inverse DWT
std::vector<float> idwt1D(const std::vector<float>& approx, const std::vector<float>& detail) {
    int N = approx.size();
    std::vector<float> result(N * 2, 0.0f);
    for (int k = 0; k < N; ++k) {
        for (int j = 0; j < 4; ++j) {
            int i = 2 * k + j;
            if (i < result.size())
                result[i] += h[j] * approx[k] + g[j] * detail[k];
        }
    }
    return result;
}

// 2D DWT (db4)
void dwt2D_db4(const std::vector<std::vector<float>>& input,
               std::vector<std::vector<float>>& LL,
               std::vector<std::vector<float>>& LH,
               std::vector<std::vector<float>>& HL,
               std::vector<std::vector<float>>& HH) {
    int h = input.size();
    int w = (h > 0) ? input[0].size() : 0;

    // Check all rows are the same size
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i].size() != static_cast<size_t>(w)) {
            std::cerr << "DWT input row " << i << " size " << input[i].size()
                      << " does not match expected " << w << std::endl;
            return;
        }
    }

    std::cout << "[DEBUG] h: " << h << " w: " << w << " (h%2=" << (h%2) << ", w%2=" << (w%2) << ")" << std::endl;

    // Add this debug print here
    for (size_t i = 0; i < input.size(); ++i) {
        std::cout << "[DEBUG] Row " << i << " size: " << input[i].size() << std::endl;
    }

    if (h % 2 != 0 || w % 2 != 0) {
        std::cerr << "Error: Input dimensions must be even for DWT. h=" << h << " w=" << w << std::endl;
        return;
    }

    std::vector<std::vector<float>> lowRows(h), highRows(h);

    // First pass: row-wise DWT
    for (int i = 0; i < h; ++i) {
        dwt1D(input[i], lowRows[i], highRows[i]);
    }

    int halfH = h / 2, halfW = w / 2;
    LL.assign(halfH, std::vector<float>(halfW));
    LH.assign(halfH, std::vector<float>(halfW));
    HL.assign(halfH, std::vector<float>(halfW));
    HH.assign(halfH, std::vector<float>(halfW));

    // Second pass: column-wise DWT
    for (int j = 0; j < halfW; ++j) {
        std::vector<float> colL(h), colH(h);
        for (int i = 0; i < h; ++i) {
            colL[i] = lowRows[i][j];
            colH[i] = highRows[i][j];
        }

        std::vector<float> La, Ld, Ha, Hd;
        dwt1D(colL, La, Ld);
        dwt1D(colH, Ha, Hd);

        for (int i = 0; i < halfH; ++i) {
            LL[i][j] = La[i];
            LH[i][j] = Ha[i];
            HL[i][j] = Ld[i];
            HH[i][j] = Hd[i];
        }
    }
}

// 2D inverse DWT (db4)
std::vector<std::vector<float>> idwt2D_db4(const std::vector<std::vector<float>>& LL,
                                           const std::vector<std::vector<float>>& LH,
                                           const std::vector<std::vector<float>>& HL,
                                           const std::vector<std::vector<float>>& HH) {
    if (LL.empty() || LL[0].empty() ||
        LH.size() != LL.size() || HL.size() != LL.size() || HH.size() != LL.size() ||
        LH[0].size() != LL[0].size() || HL[0].size() != LL[0].size() || HH[0].size() != LL[0].size()) {
        std::cerr << "Error: Subbands must be non-empty and of equal dimensions." << std::endl;
        return {};
    }

    int h = LL.size();
    int w = LL[0].size();
    std::vector<std::vector<float>> lowRows(2 * h, std::vector<float>(w));
    std::vector<std::vector<float>> highRows(2 * h, std::vector<float>(w));

    // First pass: column-wise inverse DWT
    for (int j = 0; j < w; ++j) {
        std::vector<float> colLL(h), colLH(h), colHL(h), colHH(h);
        for (int i = 0; i < h; ++i) {
            colLL[i] = LL[i][j];
            colHL[i] = HL[i][j];
            colLH[i] = LH[i][j];
            colHH[i] = HH[i][j];
        }

        std::vector<float> lowCol = idwt1D(colLL, colHL);
        std::vector<float> highCol = idwt1D(colLH, colHH);

        for (int i = 0; i < 2 * h; ++i) {
            lowRows[i][j] = lowCol[i];
            highRows[i][j] = highCol[i];
        }
    }

    // Second pass: row-wise inverse DWT
    std::vector<std::vector<float>> output(2 * h, std::vector<float>(2 * w));
    for (int i = 0; i < 2 * h; ++i) {
        if (lowRows[i].size() != static_cast<size_t>(w) || highRows[i].size() != static_cast<size_t>(w)) {
            std::cerr << "Error: Inconsistent row sizes during reconstruction." << std::endl;
            return {};
        }
        output[i] = idwt1D(lowRows[i], highRows[i]);
    }

    return output;
}