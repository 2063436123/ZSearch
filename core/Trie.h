#pragma once

#include <shared_mutex>

const int CHAR_LIMIT = 0xFF; // 字符的编码值上限
struct TrieNode
{
    TrieNode(TrieNode *parent_, int value_, bool is_word_) : parent(parent_), value(value_), is_word(is_word_)
    {
    }

//    std::string string() const
//    {
//        return "[ " + std::to_string(parent->value) + " - " + std::to_string(value) + " - " + std::to_string(is_word) +  " ]";
//    }

    bool is_word;
    int value;
    TrieNode *parent;
    std::unordered_map<int, TrieNode *> children;
};

// 使用单词🌲，实现部分匹配，例如文本中没有 su，但是输入 su 可以匹配到 sun
class Trie
{
public:
    Trie()
    {
        root = new TrieNode(nullptr, nextNodeId(), false);
    }

    void add(const std::string &word)
    {
        std::lock_guard lg(roots_lock);
        if (word.empty())
            return;
        TrieNode *node = root;

        for (size_t i = 0; i < word.size(); i++)
        {
            assert((int) word[i] >= -0xFF && (int) word[i] <= CHAR_LIMIT);
            auto &node_ref = node->children[word[i]];

            if (node_ref == nullptr)
            {
                node_ref = new TrieNode(node, nextNodeId(), false);
            }
            assert(node_ref);

            node = node->children[word[i]];
        }
        node->is_word = true;
    }

    void remove(const std::string &word)
    {
        std::lock_guard lg(roots_lock);
        if (word.empty())
            return;
        TrieNode *node = root;

        for (size_t i = 0; i < word.size(); i++)
        {
            assert((int) word[i] >= -0xFF && (int) word[i] <= CHAR_LIMIT);
            auto &node_ref = node->children[word[i]];
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

        std::vector<std::string> suffixs{""};
        tryMatchSuffix(node, suffixs, expected_num);
        if (suffixs.size() > 1)
            suffixs.pop_back();

        std::vector<std::string> res;
        for (const auto &suffix : suffixs)
            res.push_back(word + suffix);

        return res;
    }

    void clear()
    {
        std::lock_guard lg(roots_lock);
        node_id = 0;
        safe_delete();
        root = nullptr;
    }

    std::string print() const
    {
        std::shared_lock lg(roots_lock);
        std::stringstream oss;
        drawTree(oss, root);
        return oss.str();
    }

    ~Trie()
    {
        safe_delete();
    }

private:
    // TODO: delete
    void safe_delete()
    {

    }

    void drawTree(std::ostream &oss, const TrieNode *node, std::string prefix = "", bool is_tail = true,
                  char edge_char = '-') const
    {
        oss << prefix;
        oss << (is_tail ? "└" + std::string(1, edge_char) + "- " : "├" + std::string(1, edge_char) + "- ");
        oss << node->value << (node->is_word ? " (word)" : "") << '\n';

        std::string new_prefix = prefix;
        new_prefix += is_tail ? "   " : "│  ";

        int child_count = node->children.size();
        for (auto it = node->children.begin(); it != node->children.end(); ++it)
        {
            bool child_is_tail = (--child_count == 0);
            drawTree(oss, it->second, new_prefix, child_is_tail, static_cast<char>(it->first));
        }
    }

    uint64_t nextNodeId()
    {
        return node_id++;
    }

    void tryMatchSuffix(const TrieNode *node, std::vector<std::string> &suffixs, int &left_suffix_num) const
    {
        if (node == nullptr || left_suffix_num == 0)
            return;

        if (node->is_word /*&& std::all_of(suffixs.back().begin(), suffixs.back().end(), [](char ch) { return Poco::Ascii::isPrintable(ch); })*/)
        {
            suffixs.push_back(suffixs.back());
            if (--left_suffix_num == 0)
                return;
        }

        for (const auto &pair : node->children)
        {
            suffixs.back().push_back((char) pair.first);
            tryMatchSuffix(pair.second, suffixs, left_suffix_num);
            suffixs.back().pop_back();
        }
    }

    mutable std::shared_mutex roots_lock;
    std::atomic_uint64_t node_id = 0;
    TrieNode *root;
};