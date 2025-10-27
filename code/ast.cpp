#include "jsonGraph.hpp"
#include "dimacs.hpp"
#include "parsing.hpp"

#include <bits/stdc++.h>
#include <algorithm>

int main() {
    //Input
    std::string f_string;
    std::string g_string;
    std::cout << "Rules:\n() allowed\nletters for Atoms\n~ = NOT\n& = AND\n| = OR\n" << std::endl;
    std::cout << "Input 2 formulas you want to check:" << std::endl;
    std::getline(std::cin, f_string);
    std::getline(std::cin, g_string);

    //Parsing f
    Parser p_f(f_string);
    FormulaPtr f = p_f.parse_string_into_formula();
    
    //Parsing g
    Parser p_g(g_string);
    FormulaPtr g = p_g.parse_string_into_formula();

    //Graph showcase
    JsonGraph f_graph;
    auto f_data = f_graph.to_json(f);
    std::ofstream("f_graph.json") << f_data.dump(4);

    JsonGraph g_graph;
    auto g_data = g_graph.to_json(g);
    std::ofstream("g_graph.json") << g_data.dump(4);

    //P miter Q
    FormulaPtr miter = make_miter(f, g);
    NormalForm miter_cnf = cnf(miter);
    make_dimacs("answer", miter_cnf);

    return 0;
}