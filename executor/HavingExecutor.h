#pragma once

#include "Executor.h"

#include <utility>
#include "ConjunctionTree.h"
#include "utils/DynamicBitSet.h"
#include "Predicate.h"

/*
having : where
    | having 'AND' having
    | having 'OR' having
    | 'NOT' having
    | '(' having ')'
    ;
*/
class HavingExecutor : public Executor
{
public:
    HavingExecutor(Database& db_, ConjunctionTree root_ = nullptr) : Executor(db_), root(std::move(root_)) {}

    std::any execute(const std::any &doc_ids_) const override
    {
        if (!root) // output all get doc_ids
        {
            return doc_ids_;
        }
        auto doc_ids = std::any_cast<DocIds>(doc_ids_);
        DocIds ret;
        for (size_t doc_id : doc_ids)
        {
            auto document_ptr = db.findDocument(doc_id);
            if (!document_ptr)
                continue;

            auto kvs = document_ptr->getKvs();
            if (determinePredicate(kvs, root.ptr()))
                ret.emplace(doc_id);
        }
        return ret;
    }

private:
    bool determinePredicate(const std::unordered_map<Key, Value>& kvs, const ConjunctionNode *node) const
    {
        if (auto leaf = dynamic_cast<const LeafNode<Predicate>*>(node))
        {
            assert(leaf->children.empty());
            return leaf->data.determine(kvs);
        }
        else if (auto inter = dynamic_cast<const InterNode*>(node))
        {
            std::vector<bool> children_doc_ids;
            for (ConjunctionNode *child_node : node->children)
                children_doc_ids.push_back(determinePredicate(kvs, child_node));

            assert(!children_doc_ids.empty()); // AND, OR 至少有一个操作对象
            assert(inter->type != ConjunctionType::NOT || children_doc_ids.size() == 1); // NOT 只有一个操作对象

            bool ret(children_doc_ids[0]);
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
                    ret = !ret;
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

    ConjunctionTree root;
};