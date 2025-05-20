#include "huffman.hpp"
#include <queue>
#include <sstream>
#include <stdexcept>

// Huffman Node
struct Node {
    int value;
    int freq;
    Node* left;
    Node* right;

    Node(int v, int f) : value(v), freq(f), left(nullptr), right(nullptr) {}
    Node(Node* l, Node* r) : value(-1), freq(l->freq + r->freq), left(l), right(r) {}
    ~Node() { delete left; delete right; }
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

// Generate Huffman codes
void buildTable(Node* root, const std::string& str, std::unordered_map<int, std::string>& table) {
    if (!root) return;
    if (root->value != -1) {
        table[root->value] = str.empty() ? "0" : str; // Handle single-symbol case
        return;
    }
    buildTable(root->left, str + "0", table);
    buildTable(root->right, str + "1", table);
}

// Encoding
std::string huffmanEncode(const std::vector<int>& data, std::unordered_map<int, std::string>& huffTable) {
    huffTable.clear();
    if (data.empty()) return "";

    // Count frequencies
    std::unordered_map<int, int> freq;
    for (int v : data) freq[v]++;

    // Build priority queue
    std::priority_queue<Node*, std::vector<Node*>, Compare> pq;
    for (const auto& [val, f] : freq)
        pq.push(new Node(val, f));

    // Edge case: only one unique symbol
    if (pq.size() == 1) {
        Node* only = pq.top();
        buildTable(only, "", huffTable);
        std::string encoded(data.size(), '0');
        delete only;
        return encoded;
    }

    // Build Huffman tree
    while (pq.size() > 1) {
        Node* l = pq.top(); pq.pop();
        Node* r = pq.top(); pq.pop();
        pq.push(new Node(l, r));
    }

    Node* root = pq.top();
    buildTable(root, "", huffTable);

    // Encode data
    std::string encoded;
    encoded.reserve(data.size() * 2); // Reserve space for speed
    for (int v : data)
        encoded += huffTable[v];

    delete root;
    return encoded;
}

// Decoding
std::vector<int> huffmanDecode(const std::string& encoded, const std::unordered_map<std::string, int>& reverseTable, size_t expectedSymbols) {
    std::vector<int> result;
    std::string current;
    for (char bit : encoded) {
        current += bit;
        auto it = reverseTable.find(current);
        if (it != reverseTable.end()) {
            result.push_back(it->second);
            current.clear();
            if (result.size() == expectedSymbols) break;
        }
    }
    return result;
}