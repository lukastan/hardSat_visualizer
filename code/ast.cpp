#include "jsonGraph.hpp"
#include "dimacs.hpp"
#include "parsing.hpp"

#include <bits/stdc++.h>
#include <algorithm>

int main() {
    std::string f_string;
    std::cout << "Rules:\n() allowed\nletters for Atoms\n~ = NOT\n& = AND\n| = OR\n" << std::endl;
    std::cout << "Input a formula:" << std::endl;
    std::getline(std::cin, f_string);

    Parser p(f_string);
    FormulaPtr f = p.parse_string_into_formula();
    std::cout << print(f) << std::endl;

    //Graph showcase
    JsonGraph g;
    auto data = g.to_json(f);
    std::ofstream("graph.json") << data.dump(4);

    //CNF
    NormalForm our_cnf = cnf(f);
    print(our_cnf);
    std::cout << std::endl;

    //P miter Q
    FormulaPtr miter = make_miter(f, f);
    NormalForm miter_cnf = cnf(miter);
    make_dimacs("test", miter_cnf);

    return 0;
}