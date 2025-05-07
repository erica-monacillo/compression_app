#include "huffman.hpp"
#include <bitset>

std::string huffmanEncode(const std::vector<int>& data) {
    std::string output;
    for (int val : data) {
        output += std::bitset<8>(val).to_string();
    }
    return output;
}

std::vector<int> huffmanDecode(const std::string& encoded) {
    std::vector<int> decoded;
    for (size_t i = 0; i < encoded.size(); i += 8) {
        std::bitset<8> bits(encoded.substr(i, 8));
        decoded.push_back((int)bits.to_ulong());
    }
    return decoded;
}
