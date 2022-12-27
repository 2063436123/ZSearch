#pragma once

#include <utility>

#include "typedefs.h"
#include "storage/Reader.h"

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
        auto line = reader->readUntil();
        if (line.str.empty())
            return ExtractResult{};
        StringInFiles res;

        // this split logic is referred to Poco::StringTokenizer.
        auto str = line.str;
        std::string separators(" ,.\t\n");
        size_t offset_in_file = line.offset_in_file;

        auto begin = str.begin(), end = str.end();
        auto it = str.begin();

        std::string token;
        for (; it != end; ++it)
        {
            if (separators.find(*it) != std::string::npos)
            {
                size_t old_token_size = token.size();
                size_t left_trim_number = trimInPlace(token, [](char ch) {
                    return Poco::Ascii::isSpace(ch) || !Poco::Ascii::isPrintable(ch);
                }).first;
                if (!token.empty()) res.emplace_back(token, offset_in_file + (it - begin) - old_token_size + left_trim_number);
                token.clear();
            }
            else
            {
                token += *it;
            }
        }

        if (!token.empty())
        {
            size_t old_token_size = token.size();
            size_t left_trim_number = trimInPlace(token, [](char ch) {
                return Poco::Ascii::isSpace(ch) || !Poco::Ascii::isPrintable(ch);
            }).first;
            if (!token.empty()) res.emplace_back(token, offset_in_file + (it - begin) - old_token_size + left_trim_number);
        }

        return ExtractResult{.is_valid = true, .words = res};
    }

private:
    std::unique_ptr<Reader> reader;
};

class CSVExtractor : public Extractor {
public:
    explicit CSVExtractor(std::string table_name_, std::unique_ptr<Reader> reader_) : table_name(std::move(table_name_)), reader(std::move(reader_)) {}

    ExtractResult extract() override
    {
        Rows rows;
        // read column name
        auto line = reader->readUntil();
        if (line.str.empty())
            return ExtractResult{};

        Poco::StringTokenizer name_tokenizer(line.str, ",", Poco::StringTokenizer::Options::TOK_TRIM);
        size_t column_count = name_tokenizer.count();

        // read column type
        line = reader->readUntil();
        if (line.str.empty())
            throw Poco::DataFormatException("format error in file " + table_name);
        Poco::StringTokenizer type_tokenizer(line.str, ",", Poco::StringTokenizer::Options::TOK_TRIM);
        if (type_tokenizer.count() != column_count)
            throw Poco::DataFormatException("format error in file " + table_name);
        for (size_t i = 0; i < type_tokenizer.count(); i++)
        {
            auto type = getTypeByName(type_tokenizer[i]);
            switch (type)
            {
                case ColumnType::Int:
                    rows.addColumn(std::make_shared<ColumnInt>(name_tokenizer[i]));
                    break;
                case ColumnType::Decimal:
                    rows.addColumn(std::make_shared<ColumnDecimal>(name_tokenizer[i]));
                    break;
                case ColumnType::String:
                    rows.addColumn(std::make_shared<ColumnString>(name_tokenizer[i]));
                    break;
                case ColumnType::DateTime:
                    rows.addColumn(std::make_shared<ColumnDateTime>(name_tokenizer[i]));
                    break;
                default:
                    throw Poco::NotImplementedException("unknown type name in csv extractor");
            }
        }

        // read row values
        while (true)
        {
            line = reader->readUntil();
            if (line.str.empty())
                break;
            Poco::StringTokenizer value_tokenizer(line.str, ",", Poco::StringTokenizer::Options::TOK_TRIM);
            if (value_tokenizer.count() != column_count)
                throw Poco::DataFormatException("format error in file " + table_name);

            int i = 0;
            for (const auto& column_value : value_tokenizer)
            {
                if (column_value.empty())
                {
                    switch (rows.getColumns()[i]->type)
                    {
                        case ColumnType::Int:
                            std::dynamic_pointer_cast<ColumnInt>(rows.getColumns()[i])->data->insertNull();
                            break;
                        case ColumnType::String:
                            std::dynamic_pointer_cast<ColumnString>(rows.getColumns()[i])->data->insertNull();
                            break;
                        case ColumnType::Decimal:
                            std::dynamic_pointer_cast<ColumnDecimal>(rows.getColumns()[i])->data->insertNull();;
                            break;
                        case ColumnType::DateTime:
                            std::dynamic_pointer_cast<ColumnDateTime>(rows.getColumns()[i])->data->insertNull();;
                            break;
                    }
                }
                else{
                    switch (rows.getColumns()[i]->type)
                    {
                        case ColumnType::Int:
                            std::dynamic_pointer_cast<ColumnInt>(rows.getColumns()[i])->data->insert(restrictStoi<Int>(column_value));
                            break;
                        case ColumnType::String:
                            std::dynamic_pointer_cast<ColumnString>(rows.getColumns()[i])->data->insert(trimQuote(column_value));
                            break;
                        case ColumnType::Decimal:
                            std::dynamic_pointer_cast<ColumnDecimal>(rows.getColumns()[i])->data->insert(restrictStod(column_value));
                            break;
                        case ColumnType::DateTime:
                            std::dynamic_pointer_cast<ColumnDateTime>(rows.getColumns()[i])->data->insert(DateTime(column_value.substr(0, 19)));
                            break;
                    }
                }
                i++;
            }
        }
        if (rows.size() == 0)
            return ExtractResult{};

        return ExtractResult{.is_valid = true, .table = Table(table_name, rows)};
    }

private:
    std::string table_name;
    std::unique_ptr<Reader> reader;
};