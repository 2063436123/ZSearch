#pragma once
#include "../typedefs.h"
#include "core/Value.h"

// assert: Predicate 保证调用 AggregateFunction 时，value.isArray() 成立.

Value sumFunction(const Value& value)
{
    if (!value.isNumber())
        THROW(Poco::InvalidArgumentException("sumFunction() only handle NumberArray type"));

    Number init = 0;
    value.doArrayHandler<Number>([&init](std::vector<Number>* vec){
        for (Number number : *vec)
            init += number;
    });
    return init;
}

Value countFunction(const Value& value)
{
    if (value.isNull())
        THROW(Poco::InvalidArgumentException("countFunction() can't handle NullArray type"));

    int cnt = 0;
    value.doArrayHandler([&cnt](std::vector<Bool>* vec){ cnt = vec->size(); },
                         [&cnt](std::vector<Number>* vec){ cnt = vec->size(); },
                         [&cnt](std::vector<String>* vec){ cnt = vec->size(); },
                         [&cnt](std::vector<DateTime>* vec){ cnt = vec->size(); });
    return cnt;
}

Value avgFunction(const Value& value)
{
    if (!value.isNumber())
        THROW(Poco::InvalidArgumentException("avgFunction() only handle NumberArray type"));

    Value sum = sumFunction(value);
    Value count = countFunction(value);
    return sum.as<Number>() / count.as<Number>();
}

Value maxFunction(const Value& value)
{
    if (!(value.isNumber() || value.isString() || value.isDateTime()))
        THROW(Poco::InvalidArgumentException("maxFunction() only handle Number/String/DateTime Array type"));
    Value max;

    value.doArrayHandler(PanicValueArrayHandler<Bool>,
                         [&max](std::vector<Number>* vec) {
                             if (vec->empty())
                                 return;
                             max = vec->at(0);
                             for (int i = 1; i < vec->size(); i++)
                                 if (max < vec->at(i))
                                     max = vec->at(i);
                         },
                         [&max](std::vector<String>* vec) {
                             if (vec->empty())
                                 return;
                             max = vec->at(0);
                             for (int i = 1; i < vec->size(); i++)
                                 if (max < vec->at(i))
                                     max = vec->at(i);
                         },
                         [&max](std::vector<DateTime>* vec) {
                             if (vec->empty())
                                 return;
                             max = vec->at(0);
                             for (int i = 1; i < vec->size(); i++)
                                 if (max < vec->at(i))
                                     max = vec->at(i);
                         });

    return max;
}

Value minFunction(const Value& value)
{
    if (!(value.isNumber() || value.isString() || value.isDateTime()))
        THROW(Poco::InvalidArgumentException("minFunction() only handle Number/String/DateTime Array type"));
    Value min;

    value.doArrayHandler(PanicValueArrayHandler<Bool>,
                         [&min](std::vector<Number>* vec) {
                             if (vec->empty())
                                 return;
                             min = vec->at(0);
                             for (int i = 1; i < vec->size(); i++)
                                 if (min > vec->at(i))
                                     min = vec->at(i);
                         },
                         [&min](std::vector<String>* vec) {
                             if (vec->empty())
                                 return;
                             min = vec->at(0);
                             for (int i = 1; i < vec->size(); i++)
                                 if (min > vec->at(i))
                                     min = vec->at(i);
                         },
                         [&min](std::vector<DateTime>* vec) {
                             if (vec->empty())
                                 return;
                             min = vec->at(0);
                             for (int i = 1; i < vec->size(); i++)
                                 if (min > vec->at(i))
                                     min = vec->at(i);
                         });

    return min;
}