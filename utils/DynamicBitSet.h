#pragma once
#include "../typedefs.h"

class DynamicBitSet {
    const uint64_t ByteNum = 8 * sizeof(uint64_t);
    const uint64_t One = 0x1;
public:
    explicit DynamicBitSet(size_t size_) : size(size_), bit_set((size + ByteNum - 1) / ByteNum) {}

    // the minimal in vec_ should >= 1, and the maximal should <= size_.
    explicit DynamicBitSet(size_t size_, const std::vector<size_t>& vec, size_t cut_begin = 0, size_t cut_end = 0) : DynamicBitSet(size_)
    {
        if (!cut_end)
        {
            for (size_t i : vec)
                set(i);
        }
        else
        {
            for (size_t i = cut_begin; i < cut_end && i < vec.size(); i++)
            {
                set(vec[i]);
            }
        }
    }

    DynamicBitSet& operator=(const DynamicBitSet& rhs)
    {
        if (size != rhs.size)
            THROW(Poco::InvalidArgumentException());
        bit_set = rhs.bit_set;
        return *this;
    }

    DynamicBitSet& fill()
    {
        std::fill(bit_set.begin(), bit_set.end(), UINT64_MAX);
        return *this;
    }

    // cut_index starts from 0.
    DynamicBitSet& fill_range(size_t cut_begin, size_t cut_end)
    {
        for (size_t i = cut_begin; i < cut_end && i < size; i++)
        {
            set(i + 1);
        }
        return *this;
    }

    DynamicBitSet& flip()
    {
        std::transform(bit_set.begin(), bit_set.end(), bit_set.begin(), [](uint64_t v) { return ~v; });
        return *this;
    }

    // 设置第 i 位为true，i start from 1.
    DynamicBitSet& set(size_t i)
    {
        assert(i >= 1);
        i -= 1;

        size_t index = i / ByteNum;
        size_t rem = i % ByteNum;
        bit_set[index] |= (One << rem);

        return *this;
    }

    std::set<size_t> toSet(uint64_t start_index = 0) const
    {
        std::set<size_t> ret;
        for (size_t value : toVector(start_index))
            ret.emplace(value);
        return ret;
    }

    std::unordered_set<size_t> toUnorderedSet(uint64_t start_index = 0) const
    {
        std::unordered_set<size_t> ret;
        for (size_t value : toVector(start_index))
            ret.emplace(value);
        return ret;
    }

    // default return set range: [0, size-1].
    // it become [1, size] when start_index == 1.
    std::vector<size_t> toVector(uint64_t start_index = 0) const
    {
        std::vector<size_t> ret;
        for (size_t i = 0; i < bit_set.size(); i++)
        {
            uint64_t max_k_incr = ByteNum;
            if (i == bit_set.size() - 1)  // 最后一组只检查前 size % ByteNum 位，当然如果 size 是 ByteNum 的倍数，那么将检查前 ByteNum 位
                max_k_incr = (size + ByteNum - 1) % ByteNum + 1;
            uint64_t k = start_index;
            for (uint64_t j = One; j <= (One << (ByteNum - 1)) && k - start_index < max_k_incr; j <<= 1, k++)
            {
                if (j & bit_set[i])
                    ret.push_back(i * ByteNum + k);
            }
            assert(k == start_index + max_k_incr);
        }
        return ret;
    }

    DynamicBitSet& operator&=(const DynamicBitSet& rhs)
    {
        if (size != rhs.size)
            THROW(Poco::InvalidArgumentException());
        std::transform(bit_set.begin(), bit_set.end(), rhs.bit_set.begin(), bit_set.begin(), [](uint64_t a, uint64_t b) { return a & b; });
        return *this;
    }

    DynamicBitSet& operator|=(const DynamicBitSet& rhs)
    {
        if (size != rhs.size)
            THROW(Poco::InvalidArgumentException());
        std::transform(bit_set.begin(), bit_set.end(), rhs.bit_set.begin(), bit_set.begin(), [](uint64_t a, uint64_t b) { return a | b; });
        return *this;
    }

    bool empty() const
    {
        return std::all_of(bit_set.begin(), bit_set.end(), [](uint64_t v) { return v == 0; });
    }

private:
    size_t size;
    std::vector<uint64_t> bit_set;
};