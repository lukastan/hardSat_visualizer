#include "formula.hpp"

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