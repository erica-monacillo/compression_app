#pragma once
#include <vector>
#include <string>
#include <unordered_map>

std::string huffmanEncode(const std::vector<int>& data, std::unordered_map<int, std::string>& huffTable);
std::vector<int> huffmanDecode(const std::string& encoded, const std::unordered_map<std::string, int>& reverseTable);
