#ifndef FORMULA_H
#define FORMULA_H

#include <iostream>
#include <variant>
#include <memory>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <set>
#include <vector>

struct False;
struct True;
struct Atom;
struct Not;
struct Binary;

using Formula = std::variant<False, True, Atom, Not, Binary>;
using FormulaPtr = std::shared_ptr<Formula>;

struct False {};
struct True {};
struct Atom { std::string name; };
struct Not { FormulaPtr subformula; };
struct Binary {
    enum Type { And, Or, Impl, Eq } type;
    FormulaPtr left, right;
};

FormulaPtr ptr(const Formula& f) { return std::make_shared<Formula>(f); }

template<typename T>
bool is(const FormulaPtr& f) { return std::holds_alternative<T>(*f); }

template<typename T>
T as(const FormulaPtr& f) { return std::get<T>(*f); }

using Valuation = std::map<std::string, bool>;
using AtomSet = std::set<std::string>;

std::string print(const FormulaPtr& f) {
    if(is<False>(f)) return "F";
    if(is<True>(f))  return "T";
    if(is<Atom>(f))  return as<Atom>(f).name;
    if(is<Not>(f))   return "~" + print(as<Not>(f).subformula);
    if(is<Binary>(f)) {
        std::string sign;
        switch(as<Binary>(f).type) {
            case Binary::And:  sign = "&";   break;
            case Binary::Or:   sign = "|";   break;
            case Binary::Impl: sign = "->";  break;
            case Binary::Eq:   sign = "<->"; break;
        }
        return "(" + print(as<Binary>(f).left) + " " + sign + " " + print(as<Binary>(f).right) + ")";
    }
    return "";
}

bool next(Valuation& v) {
    auto it = begin(v);
    while(it != end(v) && it->second) {
        it->second = false;
        it++;
    }

    if(it == end(v))
        return false;

    return it->second = true;
}

std::vector<Valuation> getValuations(const AtomSet& atoms) {
    std::vector<Valuation> valuations;
    Valuation v;
    for(const auto& atom : atoms)
        v[atom] = false;
    do {
        valuations.push_back(v);
    } while(next(v));
    
    return valuations;
}

void getAtoms(const FormulaPtr& f, AtomSet& atoms) {
    if(is<Atom>(f))
        atoms.insert(as<Atom>(f).name);
    else if(is<Not>(f))
        getAtoms(as<Not>(f).subformula, atoms);
    else if(is<Binary>(f)) {
        getAtoms(as<Binary>(f).left, atoms);
        getAtoms(as<Binary>(f).right, atoms);
    }
}

FormulaPtr make_miter(const FormulaPtr& l, const FormulaPtr& r) {
    FormulaPtr notl = ptr(Not{l});
    FormulaPtr notr = ptr(Not{r});

    FormulaPtr half1 = ptr(Binary{Binary::And, l, notr});
    FormulaPtr half2 = ptr(Binary{Binary::And, notl, r});

    return ptr(Binary{Binary::Or, half1, half2});
}

#endif