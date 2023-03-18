#pragma once

#include "Pos.h"
#include "IAST.h"

/** Collects variants, how parser could proceed further at rightmost position.
  */
struct Expected
{
    const char *max_parsed_pos = nullptr;
    std::vector<const char *> variants;

    /// 'description' should be statically allocated string.
    void add(const char *current_pos, const char *description)
    {
        // 在解析下一个 token 时，满足 current_pos > max_parsed_pos，所以 expected 中的内容被重置为新 token 的 description.
        if (!max_parsed_pos || current_pos > max_parsed_pos)
        {
            variants.clear();
            max_parsed_pos = current_pos;
            variants.push_back(description);
            return;
        }

        if (current_pos == max_parsed_pos && std::find(variants.begin(), variants.end(), description) == variants.end())
            variants.push_back(description);
    }

    void add(TokenIterator it, const char *description)
    {
        add(it->begin, description);
    }
};

class IParser
{
public:
    virtual const char *getName() const = 0;

    /** Parse piece of text from position `pos`, but not beyond end of line (`end` - position after end of line),
      * move pointer `pos` to the maximum position to which it was possible to parse,
      * in case of success return `true` and the result in `node` if it is needed, otherwise false,
      * in `expected` write what was expected in the maximum position,
      *  to which it was possible to parse if parsing was unsuccessful,
      *  or what this parser parse if parsing was successful.
      * The string to which the [begin, end) range is included may be not 0-terminated.
      */
    virtual bool parse(Pos &pos, ASTPtr &node, Expected &expected) = 0;

    bool ignore(Pos &pos, Expected &expected)
    {
        ASTPtr ignore_node;
        return parse(pos, ignore_node, expected);
    }

    bool ignore(Pos &pos)
    {
        Expected expected;
        return ignore(pos, expected);
    }

    /** The same, but do not move the position and do not write the result to node.
     */
    bool check(Pos &pos, Expected &expected)
    {
        Pos begin = pos;
        ASTPtr node;
        if (!parse(pos, node, expected))
        {
            pos = begin;
            return false;
        }
        else
            return true;
    }

    /** The same, but doesn't move the position even if parsing was successful.
     */
    bool checkWithoutMoving(Pos pos, Expected &expected)
    {
        ASTPtr node;
        return parse(pos, node, expected);
    }

    virtual ~IParser() = default;
};

class IParserBase : public IParser
{
    template <typename F>
    static bool wrapParseImpl(Pos &pos, const F& func)
    {
        Pos begin = pos;
        bool res = func();
        if (!res)
            pos = begin;
        return res;
    }

public:
    struct IncreaseDepthTag {};

    template <typename F>
    static bool wrapParseImpl(Pos &pos, IncreaseDepthTag, const F& func)
    {
        Pos begin = pos;
        pos.increaseDepth();
        bool res = func();
        pos.decreaseDepth();
        if (!res)
            pos = begin;
        return res;
    }

    bool parse(Pos &pos, ASTPtr &node, Expected &expected) override
    {
        expected.add(pos, getName());

        return wrapParseImpl(pos, IncreaseDepthTag{}, [&]() {
           bool res = parseImpl(pos, node, expected);
           if (!res)
               node = nullptr;
           return res;
        });
    }

protected:
    virtual bool parseImpl(Pos &pos, ASTPtr &node, Expected &expected) = 0;
};
