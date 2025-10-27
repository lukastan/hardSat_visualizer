#include "formula.hpp"

struct Parser {
    std::string s;
    int pos = 0;

    explicit Parser(std::string str) : s(std::move(str)) {}

    void skip() {
        while(pos < s.size() && isspace(s[pos]))
            pos++;
    }

    bool match(char c) {
        skip();
        if(pos < s.size() && s[pos] == c) {
            pos++;
            return true;
        }

        return false;
    }

    bool peek(char c) {
        skip();
        
        if(pos < s.size() && s[pos] == c) {
            return true;
        }

        return false;
    }

    FormulaPtr parse_atom() {
        skip();
        
        if(s[pos] == '(') {
            pos++;
            auto f = parse_or();
            skip();
            pos++;

            return f;
        }

        if(isalpha(s[pos])) {
            std::string name;
            while(pos < s.size() && isalpha(s[pos])) {
                name += s[pos];
                pos++;
            }

            return ptr(Atom{name});
        }

        return FormulaPtr{};
    }

    FormulaPtr parse_string_into_formula() {
        return parse_or();
    }

    FormulaPtr parse_not() {
        if(match('~'))
            return ptr(as<Not>(parse_not()));
        return parse_atom();
    }

    FormulaPtr parse_and() {
        auto left = parse_not();
        skip();
        while(match('&')) {
            auto right = parse_not();
            left = ptr(Binary{Binary::And, left, right});
            skip();
        }

        return left;
    }

    FormulaPtr parse_or() {
        auto left = parse_and();
        skip();
        while(match('|')) {
            auto right = parse_and();
            left = ptr(Binary{Binary::Or, left, right});
            skip();
        }

        return left;
    }
};