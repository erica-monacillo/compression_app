#pragma once
#include <vector>
#include <string>

<<<<<<< Updated upstream
std::string huffmanEncode(const std::vector<int>& data);
std::vector<int> huffmanDecode(const std::string& encoded);
=======
// Returns the encoded bitstring and fills huffTable (value->bitstring)
std::string huffmanEncode(const std::vector<int>& data, std::unordered_map<int, std::string>& huffTable);

// Decodes a bitstring using a reverse table (bitstring->value)
std::vector<int> huffmanDecode(const std::string& encoded, const std::unordered_map<std::string, int>& reverseTable);
>>>>>>> Stashed changes
