#include "core/Database.h"
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
class TermsExecutor
{
public:
    TermsExecutor(Database& db_, const ConjunctionNode* root_) : db(db_), bit_set_size(0), root(root_) {}

    // return doc ids
    std::set<size_t> execute()
    {
        bit_set_size = db.maxAllocatedDocId();
        return recursiveExecute(root).toSet(1);
    }

private:
    using DocIds = DynamicBitSet;
    // return first 表示是否要 NOT 取反，second 表示要操作的集合
    DocIds recursiveExecute(const ConjunctionNode *node)
    {
        if (auto leaf = dynamic_cast<const LeafNode<std::string>*>(node))
        {
            assert(leaf->children.empty());
            auto term_ptr = db.findTerm(leaf->data);
            return DynamicBitSet(bit_set_size, term_ptr->posting_list);
        }
        else if (auto inter = dynamic_cast<const InterNode*>(node))
        {
            std::vector<DocIds> children_doc_ids;
            for (ConjunctionNode *child_node : node->children)
                children_doc_ids.push_back(recursiveExecute(child_node));

            assert(!children_doc_ids.empty()); // AND, OR 至少有一个操作对象
            assert(inter->type != ConjunctionType::NOT || children_doc_ids.size() == 1); // NOT 只有一个操作对象

            DocIds ret(children_doc_ids[0]);
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
    Database& db;
    size_t bit_set_size;
    const ConjunctionNode* root;
};