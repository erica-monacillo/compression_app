#include "dwt_db4.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

// db4 coefficients
const float h[] = {
    0.4829629131445341f,
    0.8365163037378079f,
    0.2241438680420134f,
   -0.1294095225512604f
};

const float g[] = {
   -0.1294095225512604f,
   -0.2241438680420134f,
    0.8365163037378079f,
   -0.4829629131445341f
};

// symmetric extension
std::vector<float> symmetricPad(const std::vector<float>& signal, int pad) {
    int N = signal.size();
    std::vector<float> extended(N + 2 * pad);
    for (int i = 0; i < pad; ++i) {
        extended[i] = signal[pad - i];
        extended[N + pad + i] = signal[N - 2 - i];
    }
    for (int i = 0; i < N; ++i)
        extended[i + pad] = signal[i];
    return extended;
}

void dwt1D(const std::vector<float>& input, std::vector<float>& approx, std::vector<float>& detail) {
    int n = input.size();
    std::vector<float> padded = symmetricPad(input, 2);
    approx.resize(n / 2);
    detail.resize(n / 2);
    for (int i = 0, k = 0; i < n; i += 2, ++k) {
        approx[k] = detail[k] = 0;
        for (int j = 0; j < 4; ++j) {
            approx[k] += h[j] * padded[i + j];
            detail[k] += g[j] * padded[i + j];
        }
    }
}

std::vector<float> idwt1D(const std::vector<float>& approx, const std::vector<float>& detail) {
    int n = approx.size();
    std::vector<float> result(n * 2, 0.0f);
    for (int k = 0; k < n; ++k) {
        for (int j = 0; j < 4; ++j) {
            int idx = 2 * k + j;
            if (idx < result.size()) {
                result[idx] += h[j] * approx[k] + g[j] * detail[k];
            }
        }
    }
    return result;
}

void dwt2D_db4(const std::vector<std::vector<float>>& input,
               std::vector<std::vector<float>>& LL,
               std::vector<std::vector<float>>& LH,
               std::vector<std::vector<float>>& HL,
               std::vector<std::vector<float>>& HH) {
    int h = input.size();
    int w = input[0].size();

    std::vector<std::vector<float>> row_approx(h), row_detail(h);
    for (int i = 0; i < h; ++i)
        dwt1D(input[i], row_approx[i], row_detail[i]);

    int midH = h / 2;
    int midW = w / 2;
    LL.assign(midH, std::vector<float>(midW));
    LH.assign(midH, std::vector<float>(midW));
    HL.assign(midH, std::vector<float>(midW));
    HH.assign(midH, std::vector<float>(midW));

    for (int j = 0; j < midW; ++j) {
        std::vector<float> col_a(h), col_d(h);
        for (int i = 0; i < h; ++i) {
            col_a[i] = row_approx[i][j];
            col_d[i] = row_detail[i][j];
        }

        std::vector<float> approx1, detail1, approx2, detail2;
        dwt1D(col_a, approx1, detail1);
        dwt1D(col_d, approx2, detail2);

        for (int i = 0; i < midH; ++i) {
            LL[i][j] = approx1[i];
            LH[i][j] = approx2[i];
            HL[i][j] = detail1[i];
            HH[i][j] = detail2[i];
        }
    }
}

std::vector<std::vector<float>> idwt2D_db4(const std::vector<std::vector<float>>& LL,
                                           const std::vector<std::vector<float>>& LH,
                                           const std::vector<std::vector<float>>& HL,
                                           const std::vector<std::vector<float>>& HH) {
    int h = LL.size();
    int w = LL[0].size();
    int fullH = h * 2;
    int fullW = w * 2;

    std::vector<std::vector<float>> cols(fullW, std::vector<float>(h));
    for (int j = 0; j < w; ++j) {
        std::vector<float> low = idwt1D(LL[j], HL[j]);
        std::vector<float> high = idwt1D(LH[j], HH[j]);

        for (int i = 0; i < fullH; ++i) {
            cols[2 * j][i]     = low[i];
            cols[2 * j + 1][i] = high[i];
        }
    }

    std::vector<std::vector<float>> output(fullH, std::vector<float>(fullW));
    for (int i = 0; i < fullH; ++i) {
        output[i] = idwt1D(cols[i], cols[i]); // dummy placeholder for now
    }
    return output;
}
