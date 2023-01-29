#pragma once
#include "../typedefs.h"
#include "core/Key.h"
#include "utils/json.hpp"

std::unordered_map<Key, nlohmann::json> flattenJsonKvs(const std::string& str);

std::unordered_map<Key, nlohmann::json> flattenJsonKvs(std::istream& in)
{
    std::string full_text;
    Poco::StreamCopier::copyToString64(in, full_text);
    return flattenJsonKvs(full_text);
}

void dfs(const nlohmann::json& data, Key& current_key, const std::function<void(const Key&, const nlohmann::json&)>& emit)
{
    if (data.is_object())
    {
        for (auto iter = data.begin(); iter != data.end(); ++iter)
        {
            current_key.push_back(iter.key());
            dfs(iter.value(), current_key, emit);
            current_key.pop_back();
        }
    }
    // 如果 json 中出现对象数组结构，我们只保留此数组中的对象而忽略其他 primitive 值；数组中的对象将被赋予 index 前缀
    // 对于 primitive 数组，我们将整个数组视为一个 Value
    else if (data.is_array() && std::any_of(data.begin(), data.end(), [](const nlohmann::json& data){ return data.is_object();}))
    {
        for (auto iter = data.begin(); iter != data.end(); ++iter)
        {
            if (iter->is_object())
            {
                current_key.push_back(std::to_string(iter - data.begin()));
                dfs(iter.value(), current_key, emit);
                current_key.pop_back();
            }
        }
    }
    else
    {
        emit(current_key, data);
    }
}

// flatten 后，json 的层次关系只能在 Key 中看出（Key 中 . 越多，层次越深）
std::unordered_map<Key, nlohmann::json> flattenJsonKvs(const std::string& str)
{
    std::unordered_map<Key, nlohmann::json> res;

    nlohmann::json data = nlohmann::json::parse(str);
    Key current_key("");

    dfs(data, current_key, [&res](const Key& key, const nlohmann::json& value){
        res.emplace(key, value);
    });

    return res;
}