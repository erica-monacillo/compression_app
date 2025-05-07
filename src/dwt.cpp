#include "dwt.hpp"

void dwt2D(const std::vector<std::vector<float>>& input,
           std::vector<std::vector<float>>& LL,
           std::vector<std::vector<float>>& LH,
           std::vector<std::vector<float>>& HL,
           std::vector<std::vector<float>>& HH) {
    int h = input.size();
    int w = input[0].size();
    LL = LH = HL = HH = std::vector<std::vector<float>>(h / 2, std::vector<float>(w / 2));
    for (int i = 0; i < h; i += 2) {
        for (int j = 0; j < w; j += 2) {
            float a = input[i][j];
            float b = input[i][j+1];
            float c = input[i+1][j];
            float d = input[i+1][j+1];

            LL[i/2][j/2] = (a + b + c + d) / 4;
            LH[i/2][j/2] = (a - b + c - d) / 4;
            HL[i/2][j/2] = (a + b - c - d) / 4;
            HH[i/2][j/2] = (a - b - c + d) / 4;
        }
    }
}

std::vector<std::vector<float>> idwt2D(const std::vector<std::vector<float>>& LL,
                                       const std::vector<std::vector<float>>& LH,
                                       const std::vector<std::vector<float>>& HL,
                                       const std::vector<std::vector<float>>& HH) {
    int h = LL.size();
    int w = LL[0].size();
    std::vector<std::vector<float>> output(h * 2, std::vector<float>(w * 2));

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            float a = LL[i][j] + LH[i][j] + HL[i][j] + HH[i][j];
            float b = LL[i][j] - LH[i][j] + HL[i][j] - HH[i][j];
            float c = LL[i][j] + LH[i][j] - HL[i][j] - HH[i][j];
            float d = LL[i][j] - LH[i][j] - HL[i][j] + HH[i][j];

            output[i*2][j*2] = a;
            output[i*2][j*2+1] = b;
            output[i*2+1][j*2] = c;
            output[i*2+1][j*2+1] = d;
        }
    }
    return output;
}
