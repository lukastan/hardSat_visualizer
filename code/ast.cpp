#include "jsonGraph.hpp"

using Valuation = std::map<std::string, bool>;

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
    FormulaPtr A = ptr(Atom{"A"});
    FormulaPtr B = ptr(Atom{"B"});
    FormulaPtr CIN = ptr(Atom{"CIN"});
    
    //COUT
    FormulaPtr OR1 = ptr(Binary{Binary::Or, ptr(Atom{"A"}), ptr(Atom{"B"})});
    FormulaPtr AND1 = ptr(Binary{Binary::And, ptr(Atom{"A"}), ptr(Atom{"B"})});
    FormulaPtr AND2 = ptr(Binary{Binary::And, OR1, CIN});
    FormulaPtr COUT = ptr(Binary{Binary::Or, AND1, AND2});
    
    //SUM
    FormulaPtr OR2 = ptr(Binary{Binary::Or, A, B});
    FormulaPtr AND3 = ptr(Binary{Binary::And, A, B});
    FormulaPtr multiOR = ptr(Binary{Binary::Or, OR2, CIN});
    FormulaPtr multiAND = ptr(Binary{Binary::And, AND3, CIN});
    FormulaPtr AND4 = ptr(Binary{Binary::And, multiOR, ptr(Not{COUT})});
    FormulaPtr SUM = ptr(Binary{Binary::Or, AND4, multiAND});
    
    JsonGraph g;
    std::cout << print(COUT) << std::endl;
    std::cout << print(SUM) << std::endl;
    auto sum_data = g.to_json(SUM);
    auto cout_data = g.to_json(COUT);
    std::ofstream("sum_graph.json") << sum_data.dump(4);
    std::ofstream("cout_graph.json") << cout_data.dump(4);

    return 0;
}
