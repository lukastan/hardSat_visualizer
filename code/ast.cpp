#include "jsonGraph.hpp"
#include <set>

struct Literal {
    bool pos;
    std::string name;
};

using Clause = std::vector<Literal>;
using NormalForm = std::vector<Clause>;

template<typename List>
List concat(const List& l, const List& r) {
    List result;
    std::copy(begin(l), end(l), std::back_inserter(result));
    std::copy(begin(r), end(r), std::back_inserter(result));
    return result;
}

NormalForm cross(const NormalForm& l, const NormalForm& r) {
    NormalForm result;
    for(const auto& lc : l)
        for(const auto& rc : r)
            result.push_back(concat(lc, rc));
    return result;
}

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

FormulaPtr distribute(const FormulaPtr& f) {
    if(is<Binary>(f)) {
        auto b = as<Binary>(f);
        if(b.type == Binary::Or) {
            if(is<Binary>(b.left) && as<Binary>(b.left).type == Binary::And) {
                auto bl = as<Binary>(b.left);
                return ptr(
                    Binary{Binary::And,
                    distribute(ptr(Binary{Binary::Or, bl.left, b.right})),
                    distribute(ptr(Binary{Binary::Or, bl.right, b.right}))
                });
            }
            if(is<Binary>(b.right) && as<Binary>(b.right).type == Binary::And) {
                auto br = as<Binary>(b.right);
                return ptr(
                    Binary{Binary::And,
                    distribute(ptr(Binary{Binary::Or, b.left, br.left})),
                    distribute(ptr(Binary{Binary::Or, b.left, br.right}))
                });
            }
        }

        return ptr(Binary{b.type, distribute(b.left), distribute(b.right)});
    }
    if(is<Not>(f))
        return ptr(Not{distribute(as<Not>(f).subformula)});

    return f;
}

FormulaPtr nnf(const FormulaPtr& f);

FormulaPtr nnfNot(const FormulaPtr& f) {
    if(is<True>(f))
        return nnf(ptr(False{}));
    if(is<False>(f))
        return nnf(ptr(True{}));
    if(is<Atom>(f))
        return ptr(Not{f});
    if(is<Not>(f))
        return nnf(as<Not>(f).subformula);
    auto b = as<Binary>(f);
    if(b.type == Binary::And)
        return ptr(Binary{Binary::Or, nnfNot(b.left), nnfNot(b.right)});
    if(b.type == Binary::Or)
        return ptr(Binary{Binary::And, nnfNot(b.left), nnfNot(b.right)});
    if(b.type == Binary::Impl)
        return ptr(Binary{Binary::And, nnf(b.left), nnfNot(b.right)});
    if(b.type == Binary::Eq)
        return ptr(Binary{
            Binary::Or,
            ptr(Binary{Binary::And, nnf(b.left), nnfNot(b.right)}),
            ptr(Binary{Binary::And, nnfNot(b.left), nnf(b.right)})
        });
    return FormulaPtr{};
}

FormulaPtr nnf(const FormulaPtr& f) {
   if(is<False>(f) || is<True>(f) || is<Atom>(f))
       return f;
   if(is<Not>(f))
       return nnfNot(as<Not>(f).subformula);
   auto b = as<Binary>(f);
   if(b.type == Binary::And)
       return ptr(Binary{Binary::And, nnf(b.left), nnf(b.right)});
   if(b.type == Binary::Or)
       return ptr(Binary{Binary::Or, nnf(b.left), nnf(b.right)});
   if(b.type == Binary::Impl)
       return ptr(Binary{Binary::Or, nnfNot(b.left), nnf(b.right)});
   if(b.type == Binary::Eq)
       return ptr(Binary{Binary::And,
                         ptr(Binary{Binary::Or, nnfNot(b.left), nnf(b.right)}),
                         ptr(Binary{Binary::Or, nnf(b.left), nnfNot(b.right)})
                  });
   return FormulaPtr{};
}

NormalForm cnf_rec(const FormulaPtr& f) {
    if(is<True>(f))
        return {};
    if(is<False>(f))
        return {{}};
    if(is<Atom>(f))
        return {{Literal{true, as<Atom>(f).name}}};
    if(is<Not>(f))
        return {{Literal{false, as<Atom>(as<Not>(f).subformula).name}}};
    auto b = as<Binary>(f);
    if(b.type == Binary::And)
        return concat(cnf_rec(b.left), cnf_rec(b.right));
    if(b.type == Binary::Or)
        return cross(cnf_rec(b.left), cnf_rec(b.right));
    return NormalForm{};
}

template<typename Literal>
bool operator<(Literal l, Literal r) {
    return l.name < r.name;
}

NormalForm cnf_clean(const NormalForm& cnf) {
    NormalForm cnf_clean;

    std::set<Literal> our_set;
    std::set<Clause> new_clauses;
    Clause new_clause;
    for(const auto& clause : cnf) {
        new_clause.clear();
        our_set.clear();
        for(const auto& literal : clause) {
            our_set.insert(literal);
        }
        for(Literal lit : our_set)
            new_clause.push_back(lit);
        new_clauses.insert(new_clause);
    }

    for(Clause cl : new_clauses)
        cnf_clean.push_back(cl);

    return cnf_clean;
}

NormalForm cnf(const FormulaPtr& f) {
    FormulaPtr f_nnf = nnf(f);
    FormulaPtr f_nnf_dist = distribute(f_nnf);
    NormalForm f_cnf = cnf_rec(f_nnf_dist);
    NormalForm f_cnf_clean = cnf_clean(f_cnf);
    return f_cnf_clean;
}

void print(const NormalForm& f) {
    for(const auto& clause : f) {
        std::cout << "[ ";
        for (const auto &literal : clause)
            std::cout << (literal.pos ? "" : "~") << literal.name << " ";
        std::cout << "]";
    }
    std::cout << std::endl;
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
    
    //Graph showcase
    JsonGraph g;
    std::cout << print(COUT) << std::endl;
    //std::cout << print(SUM) << std::endl;
    auto sum_data = g.to_json(SUM);
    auto cout_data = g.to_json(COUT);
    std::ofstream("sum_graph.json") << sum_data.dump(4);
    std::ofstream("cout_graph.json") << cout_data.dump(4);

    //CNF
    NormalForm cout_cnf = cnf(COUT);
    std::cout << print(nnf(COUT)) << std::endl;
    print(cout_cnf);
    // NormalForm sum_cnf = cnf(SUM);
    // print(sum_cnf);
    // std::cout << print(nnf(SUM)) << std::endl;

    return 0;
}
