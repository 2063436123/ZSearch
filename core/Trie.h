#pragma once
#include <shared_mutex>

const int CHAR_LIMIT = 0xFF; // 字符的编码值上限
struct TrieNode
{
    TrieNode(TrieNode* parent_, int value_, bool is_word_) : parent(parent_), value(value_), is_word(is_word_)
    {
    }

//    std::string string() const
//    {
//        return "[ " + std::to_string(parent->value) + " - " + std::to_string(value) + " - " + std::to_string(is_word) +  " ]";
//    }

    bool is_word;
    int value;
    TrieNode* parent;
    std::unordered_map<int, TrieNode*> children;
};

// 使用单词🌲，实现部分匹配，例如文本中没有 su，但是输入 su 可以匹配到 sun
class Trie
{
public:
    Trie()
    {
        root = new TrieNode(nullptr, nextNodeId(), false);
    }

    void add(const std::string& word)
    {
        std::lock_guard lg(roots_lock);
        if (word.empty())
            return;
        TrieNode *node = root;

        for (size_t i = 0; i < word.size(); i++)
        {
            assert((int)word[i] >= -0xFF && (int)word[i] <= CHAR_LIMIT);
            auto& node_ref = node->children[word[i]];

            if (node_ref == nullptr)
            {
                node_ref = new TrieNode(node, nextNodeId(), false);
            }
            assert(node_ref);

            node = node->children[word[i]];
        }
        node->is_word = true;
    }

    void remove(const std::string& word)
    {
        std::lock_guard lg(roots_lock);
        if (word.empty())
            return;
        TrieNode *node = root;

        for (size_t i = 0; i < word.size(); i++)
        {
            assert((int)word[i] >= -0xFF && (int)word[i] <= CHAR_LIMIT);
            auto& node_ref = node->children[word[i]];
            if (!node_ref)
                return;

            node = node->children[word[i]];
        }
        if (node)
            node->is_word = false;
    }

    std::vector<std::string> match(std::string word, int expected_num = 1) const
    {
        std::shared_lock lg(roots_lock);

        if (word.empty())
            return {word};
        auto node = root;

        for (size_t i = 0; i < word.size(); i++)
        {
            if (!node->children.contains(word[i]))
                return {word};
            node = node->children.find(word[i])->second;
        }

        std::string suffix;
        tryMatchSuffix(node, suffix);
        return {word + suffix};
    }

    ~Trie()
    {
        // TODO: delete
    }
private:
    uint64_t nextNodeId()
    {
        return node_id++;
    }

    bool tryMatchSuffix(const TrieNode* node, std::string& suffix) const
    {
        if (node == nullptr)
            return false;

        if (node->is_word)
            return true;

        for (const auto& pair : node->children)
        {
            suffix.push_back((char)pair.first);
            if (tryMatchSuffix(pair.second, suffix))
            {
                return true;
            }
            suffix.pop_back();
        }
        return false;
    }

    mutable std::shared_mutex roots_lock;
    std::atomic_uint64_t node_id = 0;
    TrieNode* root;
};