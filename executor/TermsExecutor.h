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
    std::pair<bool, std::any> execute(const std::any& input) override
    {
        const size_t bit_set_size = db.maxAllocatedDocId();

        size_t cut_num = bit_set_size;
        try {
            cut_num = std::any_cast<size_t>(input);
        } catch (const std::bad_any_cast& e) {}

        DynamicBitSet doc_ids_res(bit_set_size);
        if (!root) // output all doc_id
            doc_ids_res.fill_range(last_cut_begin, last_cut_begin + cut_num);
        else
            doc_ids_res = recursiveExecute(root.ptr(), bit_set_size, cut_num);

        if (doc_ids_res.empty())
            return {false, {}};

        last_cut_begin += cut_num;

        return {true, doc_ids_res.toUnorderedSet(1)};
    }

private:
    DynamicBitSet recursiveExecute(const ConjunctionNode *node, const size_t bit_set_size, size_t cut_num)
    {
        if (auto leaf = dynamic_cast<const LeafNode<std::string>*>(node))
        {
            assert(leaf->children.empty());
            auto term_ptr = db.findTerm(leaf->data);
            if (!term_ptr)
                return DynamicBitSet(bit_set_size);

            return DynamicBitSet(bit_set_size, term_ptr->posting_list, last_cut_begin, last_cut_begin + cut_num);
        }
        else if (auto inter = dynamic_cast<const InterNode*>(node))
        {
            std::vector<DynamicBitSet> children_doc_ids;
            for (ConjunctionNode *child_node : node->children)
                children_doc_ids.push_back(recursiveExecute(child_node, bit_set_size, cut_num));

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

    void clear() override
    {
        last_cut_begin = 0;
    }
private:
    ConjunctionTree root;
    // 只适用于 vecotrization model + single term 场景下，不支持 conjunction term 查询（幸好queryparser也不支持）
    size_t last_cut_begin = 0;
};