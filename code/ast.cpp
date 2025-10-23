#include "jsonGraph.hpp"
#include <bits/stdc++.h>
#include <algorithm>
#include <set>

struct Literal {
    bool pos;
    std::string name;

    bool operator==(Literal const& o) const noexcept {
        return pos == o.pos && name == o.name;
    }

    bool operator<(Literal const& o) const noexcept {
        if (name != o.name)
            return name < o.name;
        return pos < o.pos;
    }
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

FormulaPtr simplify(const FormulaPtr& f) {
    if(is<False>(f) || is<True>(f) || is<Atom>(f))
        return f;
    if(is<Not>(f)) {
        FormulaPtr s = simplify(as<Not>(f).subformula);
        if(is<True>(s))
            return ptr(False{});
        if(is<False>(s))
            return ptr(True{});
        return ptr(Not{s});
    }
    auto b = as<Binary>(f);
    FormulaPtr ls = simplify(b.left);
    FormulaPtr rs = simplify(b.right);
    if(b.type == Binary::And) {
        if(is<False>(ls) || is<False>(rs))
            return ptr(False{});
        if(is<True>(ls))
            return rs;
        if(is<True>(rs))
            return ls;
        return ptr(Binary{Binary::And, ls, rs});
    }
    if(b.type == Binary::Or) {
        if(is<True>(ls) || is<True>(rs))
            return ptr(True{});
        if(is<False>(ls))
            return rs;
        if(is<False>(rs))
            return ls;
        return ptr(Binary{Binary::Or, ls, rs});
    }
    if(b.type == Binary::Impl) {
        if(is<False>(ls) || is<True>(rs))
            return ptr(True{});
        if(is<True>(ls))
            return rs;
        if(is<False>(rs))
            return ptr(Not{ls});
        return ptr(Binary{Binary::Impl, ls, rs});
    }
    if(b.type == Binary::Eq) {
        if(is<True>(ls))
            return rs;
        if(is<True>(rs))
            return ls;
        if(is<False>(ls) && is<False>(rs))
            return ptr(True{});
        if(is<False>(ls))
            return ptr(Not{rs});
        if(is<False>(rs))
            return ptr(Not{ls});
        return ptr(Binary{Binary::Eq, ls, rs});
    }
    return FormulaPtr{};
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

NormalForm cnf_clean(const NormalForm& cnf) {
    NormalForm cnf_cleaned;

    for(auto &c : cnf) {
        std::set<std::pair<std::string, bool>> literals;
        bool taut = false;

        for(auto &l : c) {
            auto neg = std::make_pair(l.name, !l.pos);
            if(literals.count(neg)) {
                taut = true;
                break;
            }
            literals.insert({l.name, l.pos});
        }

        if(taut)
            continue;

        Clause new_clause;
        for(auto& [name, pos] : literals)
            new_clause.push_back({pos, name});
        cnf_cleaned.push_back(new_clause);
    }

    return cnf_cleaned;
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

void make_dimacs(std::string name, NormalForm cnf) {
    std::map<std::string, int> literal_map;
    int n_clauses = 0;
    int i = 1;
    for(auto &clause : cnf) {
        for(auto &literal : clause) {
            if(!literal_map.count(literal.name)) {
                literal_map[literal.name] = i;
                i++;
            }
        }
        n_clauses++;
    }

    int n_literals = literal_map.size();
    std::string output_string = "p cnf " + std::to_string(n_literals) + " " + std::to_string(n_clauses) + "\n";
    std::string one_clause = "";
    for(auto &clause : cnf) {
        one_clause = "";
        for(auto lit : clause) {
            if(!lit.pos)
                one_clause.append("-");
            one_clause += std::to_string(literal_map.at(lit.name));
            one_clause.append(" ");
        }
        one_clause += "0\n";
        output_string += one_clause;
    }

    std::ofstream(name + ".cnf") << output_string;
}

FormulaPtr make_miter(const FormulaPtr& l, const FormulaPtr& r) {
    FormulaPtr notl = ptr(Not{l});
    FormulaPtr notr = ptr(Not{r});

    FormulaPtr half1 = ptr(Binary{Binary::And, l, notr});
    FormulaPtr half2 = ptr(Binary{Binary::And, notl, r});

    return ptr(Binary{Binary::Or, half1, half2});
}

int main() {
    FormulaPtr A = ptr(Atom{"A"});
    FormulaPtr B = ptr(Atom{"B"});
    FormulaPtr CIN = ptr(Atom{"CIN"});
    
    //COUT 1

    FormulaPtr OR1 = ptr(Binary{Binary::Or, ptr(Atom{"A"}), ptr(Atom{"B"})});
    FormulaPtr AND1 = ptr(Binary{Binary::And, ptr(Atom{"A"}), ptr(Atom{"B"})});
    FormulaPtr AND2 = ptr(Binary{Binary::And, OR1, CIN});
    FormulaPtr COUT = ptr(Binary{Binary::Or, AND1, AND2});
    
    //COUT 2
    FormulaPtr and1 = ptr(Binary{Binary::And, A, B});
    FormulaPtr and2 = ptr(Binary{Binary::And, A, CIN});
    FormulaPtr and3 = ptr(Binary{Binary::And, B, CIN});
    FormulaPtr or1 = ptr(Binary{Binary::Or, and1, and2});
    FormulaPtr cout2 = ptr(Binary{Binary::Or, or1, and3});

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
    NormalForm COUT_cnf = cnf(COUT);
    print(COUT_cnf);
    std::cout << std::endl;
    // NormalForm sum_cnf = cnf(SUM);
    // print(sum_cnf);
    // std::cout << print(nnf(SUM)) << std::endl;
    // std::cout << std::endl;
    std::cout << print(cout2) << std::endl;
    NormalForm cout_cnf = cnf(cout2);
    print(cout_cnf);
    std::cout << std::endl;


    //P miter Q
    FormulaPtr miter = make_miter(COUT, cout2);
    NormalForm our_cnf = cnf(miter);
    make_dimacs("test", our_cnf);

    return 0;
}
