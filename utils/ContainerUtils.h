#include "../typedefs.h"

// 联合排序两个数组：排序 A 数组，然后根据 A 数组排序结果同步 B 数组的元素顺序
template<typename T, typename U>
auto syncSort(std::vector<T> oa, std::vector<U> ob)
{
    // TODO: 不使用额外数组，完成原地 sync_sort
    auto a = oa, b = ob;
    assert(a.size() == b.size());

    std::vector<std::pair<T, size_t>> base_array;
    for (size_t i = 0; i < a.size(); i++)
    {
        base_array.emplace_back(a[i], i);
    }

    std::sort(base_array.begin(), base_array.end(), [](const std::pair<T, size_t>& lhs, const std::pair<T, size_t>& rhs) {
       return lhs.first < rhs.first;
    });

    for (size_t i = 0; i < base_array.size(); i++)
    {
        size_t index1 = i;
        size_t index2 = base_array[i].second;
//        std::cout << index1 << " " << index2 << std::endl;
        a[index1] = oa[index2];
        b[index1] = ob[index2];
//        std::swap(a[index1], a[index2]);
//        std::swap(b[index1], b[index2]);
    }

    return std::make_pair(a, b);
}

// 删除 A 数组中位于 index 位置上的元素
template<typename T>
void removeElements(std::vector<T>& oa, const std::unordered_set<size_t>& index_to_delete)
{
    if (index_to_delete.empty())
        return;
    assert(*std::max_element(index_to_delete.begin(), index_to_delete.end()) < oa.size());

    int j = 0;

    for (size_t i = 0; i < oa.size(); i++)
    {
        if (!index_to_delete.contains(i)) // 如果可以保留 i
        {
            oa[j] = oa[i];
            j++;
        }
    }

    oa.erase(oa.end() - index_to_delete.size(), oa.end());
}