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

//class CSVExtractor : public Extractor {
//public:
//    explicit CSVExtractor(std::string table_name_, std::unique_ptr<Reader> reader_) : table_name(std::move(table_name_)), reader(std::move(reader_)) {}
//
//    ExtractResult extract() override
//    {
//        Rows rows;
//        // read column name
//        auto line = reader->readUntil();
//        if (line.str.empty())
//            return ExtractResult{};
//
//        Poco::StringTokenizer name_tokenizer(line.str, ",", Poco::StringTokenizer::Options::TOK_TRIM);
//        size_t column_count = name_tokenizer.count();
//
//        // read column type
//        line = reader->readUntil();
//        if (line.str.empty())
//            THROW(Poco::DataFormatException("format error in file " + table_name));
//        Poco::StringTokenizer type_tokenizer(line.str, ",", Poco::StringTokenizer::Options::TOK_TRIM);
//        if (type_tokenizer.count() != column_count)
//            THROW(Poco::DataFormatException("format error in file " + table_name));
//        for (size_t i = 0; i < type_tokenizer.count(); i++)
//        {
//            auto type = getTypeByName(type_tokenizer[i]);
//            switch (type)
//            {
//                case ColumnType::Int:
//                    rows.addColumn(std::make_shared<ColumnInt>(name_tokenizer[i]));
//                    break;
//                case ColumnType::Decimal:
//                    rows.addColumn(std::make_shared<ColumnDecimal>(name_tokenizer[i]));
//                    break;
//                case ColumnType::String:
//                    rows.addColumn(std::make_shared<ColumnString>(name_tokenizer[i]));
//                    break;
//                case ColumnType::DateTime:
//                    rows.addColumn(std::make_shared<ColumnDateTime>(name_tokenizer[i]));
//                    break;
//                default:
//                    THROW(Poco::NotImplementedException("unknown type name in csv extractor"));
//            }
//        }
//
//        // read row values
//        while (true)
//        {
//            line = reader->readUntil();
//            if (line.str.empty())
//                break;
//            Poco::StringTokenizer value_tokenizer(line.str, ",", Poco::StringTokenizer::Options::TOK_TRIM);
//            if (value_tokenizer.count() != column_count)
//                THROW(Poco::DataFormatException("format error in file " + table_name));
//
//            int i = 0;
//            for (const auto& column_value : value_tokenizer)
//            {
//                if (column_value.empty())
//                {
//                    switch (rows.getColumns()[i]->type)
//                    {
//                        case ColumnType::Int:
//                            std::dynamic_pointer_cast<ColumnInt>(rows.getColumns()[i])->data->insertNull();
//                            break;
//                        case ColumnType::String:
//                            std::dynamic_pointer_cast<ColumnString>(rows.getColumns()[i])->data->insertNull();
//                            break;
//                        case ColumnType::Decimal:
//                            std::dynamic_pointer_cast<ColumnDecimal>(rows.getColumns()[i])->data->insertNull();;
//                            break;
//                        case ColumnType::DateTime:
//                            std::dynamic_pointer_cast<ColumnDateTime>(rows.getColumns()[i])->data->insertNull();;
//                            break;
//                    }
//                }
//                else{
//                    switch (rows.getColumns()[i]->type)
//                    {
//                        case ColumnType::Int:
//                            std::dynamic_pointer_cast<ColumnInt>(rows.getColumns()[i])->data->insert(restrictStoi<Int>(column_value));
//                            break;
//                        case ColumnType::String:
//                            std::dynamic_pointer_cast<ColumnString>(rows.getColumns()[i])->data->insert(trimQuote(column_value));
//                            break;
//                        case ColumnType::Decimal:
//                            std::dynamic_pointer_cast<ColumnDecimal>(rows.getColumns()[i])->data->insert(restrictStod(column_value));
//                            break;
//                        case ColumnType::DateTime:
//                            std::dynamic_pointer_cast<ColumnDateTime>(rows.getColumns()[i])->data->insert(DateTime(column_value.substr(0, 19)));
//                            break;
//                    }
//                }
//                i++;
//            }
//        }
//        if (rows.size() == 0)
//            return ExtractResult{};
//
//        return ExtractResult{.is_valid = true, .table = Table(table_name, rows)};
//    }
//
//private:
//    std::string table_name;
//    std::unique_ptr<Reader> reader;
//};


class JsonExtractor : public Extractor {
public:
    explicit JsonExtractor(std::unique_ptr<Reader> reader_) : reader(std::move(reader_)) {}

    ExtractResult extract() override
    {
        StringInFiles res;
        extractWords(reader, res, " \"{}:,.\t\n");

        reader.reset();
        // TODO： 目前只提取 words，需要提取 kvs
        extractKvs(reader, res);

        if (res.empty())
            return ExtractResult{};
        return ExtractResult{.is_valid = true, .words = res};
    }

private:
    std::unique_ptr<Reader> reader;
};