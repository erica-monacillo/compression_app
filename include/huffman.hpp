#pragma once
#include <vector>
#include <string>

std::string huffmanEncode(const std::vector<int>& data);
std::vector<int> huffmanDecode(const std::string& encoded);
