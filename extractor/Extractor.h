#pragma once

#include <utility>

#include "typedefs.h"
#include "ExtractUtils.h"

class Extractor {
public:
    virtual ExtractResult extract() = 0;
    virtual ~Extractor() = default;
};

class WordExtractor : public Extractor {
public:
    explicit WordExtractor(std::unique_ptr<Reader> reader_) : reader(std::move(reader_)) {}

    ExtractResult extract() override
    {
        StringInFiles res;
        extractWords(reader, res);

        if (res.empty())
            return ExtractResult{};
        return ExtractResult{.is_valid = true, .words = res};
    }

private:
    std::unique_ptr<Reader> reader;
};

class JsonExtractor : public Extractor {
public:
    explicit JsonExtractor(std::unique_ptr<Reader> reader_) : reader(std::move(reader_)) {}

    ExtractResult extract() override
    {
        StringInFiles word_res;
        extractWords(reader, word_res, " \"{}:,.\t\n");

        std::unordered_map<Key, Value> kv_res;

        try
        {
            extractKvs(reader, kv_res);
        }
        catch (const nlohmann::json::exception& j)
        {
            httpLog(std::string("json parse error, but words are saved.") + j.what() + " file_path - " + reader->getFilePath().string());
            kv_res.clear();
        }

        if (word_res.empty())
            return ExtractResult{};
        return ExtractResult{.is_valid = true, .words = word_res, .kvs = kv_res};
    }

private:
    std::unique_ptr<Reader> reader;
};