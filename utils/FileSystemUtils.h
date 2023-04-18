#pragma once

#include "../typedefs.h"

std::unordered_set<std::string> gatherExistedFiles(const std::filesystem::path &path)
{
    if (!exists(path))
        return {};

    if (!is_directory(path)) // 如果 path 指向一个文件，那么不在判断类型是否允许 —— 直接加入索引
        return {path.string()};

    std::unordered_set<std::string> res;
    try
    {
        for (const auto &path_entry : std::filesystem::recursive_directory_iterator(path))
        {
            if (path_entry.is_regular_file() && ALLOWED_FILE_EXTENSIONS.contains(path_entry.path().extension()))
            {
                res.insert(path_entry.path().string());
            }

        }
    } catch (std::exception &e) // TODO: 解决遍历文件时的权限问题，而不是跳过之
    {
    }
    return res;
}