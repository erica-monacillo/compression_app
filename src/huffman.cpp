#include "huffman.hpp"
#include <queue>
#include <map>
#include <bitset>

struct Node {
    int value;
    int freq;
    Node* left;
    Node* right;
    Node(int v, int f) : value(v), freq(f), left(nullptr), right(nullptr) {}
    Node(Node* l, Node* r) : value(-1), freq(l->freq + r->freq), left(l), right(r) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

void buildTable(Node* node, const std::string& str, std::unordered_map<int, std::string>& table) {
    if (!node) return;
    if (!node->left && !node->right) {
        table[node->value] = str;
    }
    buildTable(node->left, str + "0", table);
    buildTable(node->right, str + "1", table);
}

void deleteTree(Node* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

std::string huffmanEncode(const std::vector<int>& data, std::unordered_map<int, std::string>& huffTable) {
    std::unordered_map<int, int> freq;
    for (int val : data) freq[val]++;

    std::priority_queue<Node*, std::vector<Node*>, Compare> pq;
    for (auto& p : freq) {
        pq.push(new Node(p.first, p.second));
    }

    while (pq.size() > 1) {
        Node* l = pq.top(); pq.pop();
        Node* r = pq.top(); pq.pop();
        pq.push(new Node(l, r));
    }

    Node* root = pq.top();
    buildTable(root, "", huffTable);

    std::string encoded;
    for (int val : data) {
        encoded += huffTable[val];
    }

    deleteTree(root);
    return encoded;
}

std::vector<int> huffmanDecode(const std::string& encoded, const std::unordered_map<std::string, int>& reverseTable) {
    std::vector<int> result;
    std::string current;
    for (char bit : encoded) {
        current += bit;
        if (reverseTable.count(current)) {
            result.push_back(reverseTable.at(current));
            current.clear();
        }
    }
    return result;
}
