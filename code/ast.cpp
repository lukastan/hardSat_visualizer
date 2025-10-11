#include "jsonGraph.hpp"

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

bool evaluate(const FormulaPtr& f, Valuation& v) {
    if(is<False>(f)) return false;
    if(is<True>(f))  return true;
    if(is<Atom>(f))  return v[as<Atom>(f).name];
    if(is<Not>(f))   return !evaluate(as<Not>(f).subformula, v);
    if(is<Binary>(f)) {
        bool L = evaluate(as<Binary>(f).left, v);
        bool R = evaluate(as<Binary>(f).right, v);
        switch(as<Binary>(f).type) {
            case Binary::And:  return L && R;
            case Binary::Or:   return L || R;
            case Binary::Impl: return !L || R;
            case Binary::Eq:   return L == R;
        }
    }
    return false;
}

int main() {
    FormulaPtr p = ptr(Atom{"p"});
    FormulaPtr q = ptr(Atom{"q"});
    FormulaPtr formula = ptr(Binary{Binary::Or, ptr(Not{p}), q});

    std::cout << print(formula) << std::endl;

    JsonGraph g;
    auto data = g.to_json(formula);
    std::ofstream("graph.json") << data.dump(2);

    return 0;
}
