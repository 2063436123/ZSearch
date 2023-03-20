#pragma once

#include "Executor.h"

#include <utility>
#include "ConjunctionTree.h"
#include "utils/DynamicBitSet.h"

/*
terms : terms 'AND' terms
    | terms 'OR' terms
    | 'NOT' terms
    | '(' terms ')'
    | term
    ;
*/
class TermsExecutor : public Executor
{
public:
    TermsExecutor(Database& db_, ConjunctionTree root_ = nullptr) : Executor(db_), root(std::move(root_)) {}

    // return doc ids
    std::any execute(const std::any&) const override
    {
        const size_t bit_set_size = db.maxAllocatedDocId();
        if (!root) // output all doc_id
        {
            return DynamicBitSet(bit_set_size).fill().toUnorderedSet(1);
        }
        return recursiveExecute(root.ptr(), bit_set_size).toUnorderedSet(1);
    }

private:
    DynamicBitSet recursiveExecute(const ConjunctionNode *node, const size_t bit_set_size) const
    {
        if (auto leaf = dynamic_cast<const LeafNode<std::string>*>(node))
        {
            assert(leaf->children.empty());
            auto term_ptr = db.findTerm(leaf->data);
            if (!term_ptr)
                return DynamicBitSet(bit_set_size);
            return DynamicBitSet(bit_set_size, term_ptr->posting_list);
        }
        else if (auto inter = dynamic_cast<const InterNode*>(node))
        {
            std::vector<DynamicBitSet> children_doc_ids;
            for (ConjunctionNode *child_node : node->children)
                children_doc_ids.push_back(recursiveExecute(child_node, bit_set_size));

            assert(!children_doc_ids.empty()); // AND, OR 至少有一个操作对象
            assert(inter->type != ConjunctionType::NOT || children_doc_ids.size() == 1); // NOT 只有一个操作对象

            DynamicBitSet ret(children_doc_ids[0]);
            switch (inter->type)
            {
                case ConjunctionType::AND:
                    for (int i = 1; i < children_doc_ids.size(); i++)
                        ret &= children_doc_ids[i];
                    break;
                case ConjunctionType::OR:
                    for (int i = 1; i < children_doc_ids.size(); i++)
                        ret |= children_doc_ids[i];
                    break;
                case ConjunctionType::NOT:
                    ret.flip();
                    break;
                default:
                    THROW(UnreachableException());
            }
            return ret;
        }
        else
        {
            THROW(Poco::LogicException("unexpected dynamic type"));
        }
        THROW(UnreachableException());
    }

private:
    ConjunctionTree root;
};