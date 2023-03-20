#pragma once

#include "../typedefs.h"

enum class ConjunctionType
{
    AND,
    OR,
    NOT
};

struct ConjunctionNode {
    virtual ~ConjunctionNode() = default;
    ConjunctionNode& addChild(ConjunctionNode* child)
    {
        children.push_back(child);
        return *this;
    }

    std::vector<ConjunctionNode*> children;
};

class ConjunctionTree
{
public:
    ConjunctionTree(ConjunctionNode* root_, bool adopt_pointer_ = false) : adopt_pointer(adopt_pointer_)
    {
        if (adopt_pointer)
            own_root = std::shared_ptr<ConjunctionNode>(root_);
        else
            root = root_;
    }

    bool operator!() const
    {
        return ptr() == nullptr;
    }

    [[nodiscard]] ConjunctionNode* ptr() const
    {
        return adopt_pointer ? own_root.get() : root;
    }

private:
    bool adopt_pointer;
    ConjunctionNode* root = nullptr;
    std::shared_ptr<ConjunctionNode> own_root;
};

struct InterNode : public ConjunctionNode {
    InterNode(ConjunctionType type_) : type(type_) {}
    ConjunctionType type;
};

// 由逻辑连接词 （AND, OR, NOT） 组合成的树
// 用于 terms 时, T 是 std::string
// 用于 having 时， T 是 Predicate
template <typename T>
struct LeafNode : public ConjunctionNode {
    LeafNode(const T& data_) : data(data_) {}
    T data;
};
