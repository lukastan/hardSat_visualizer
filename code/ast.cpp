#include "jsonGraph.hpp"
#include "dimacs.hpp"
#include "parsing.hpp"

#include <bits/stdc++.h>
#include <algorithm>
#include <cstdlib>

int main() {
    //Input
    std::string s;
    std::cout << "How many outs:" << std::endl;
    std::getline(std::cin, s);
    int n_outs = stoi(s);

    if(n_outs <= 0)
        std::cout << "Nothing to solve!" << std::endl;

    std::cout << "Rules:\n() allowed\nletters for Atoms\n~ = NOT\n& = AND\n| = OR\n" << std::endl;

    std::string f_string;
    std::string g_string;
    std::string file_name = "answer";
    std::string dimacs_file_name;
    for(int i = 0; i < n_outs; i++) {
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
        std::string f_file_name = "f_graph_" + std::to_string(i) + ".json";
        std::ofstream(f_file_name) << f_data.dump(4);

        JsonGraph g_graph;
        auto g_data = g_graph.to_json(g);
        std::string g_file_name = "g_graph_" + std::to_string(i) + ".json";
        std::ofstream(g_file_name) << g_data.dump(4);

        //P miter Q
        FormulaPtr miter = make_miter(f, g);
        NormalForm miter_cnf = cnf(miter);
        std::string dimacs_file_name = file_name + std::to_string(i);
        make_dimacs(dimacs_file_name, miter_cnf);
    }

    //Pipe to minisat
    for(int j = 0; j < n_outs; j++) {
        std::string num = std::to_string(j);
        dimacs_file_name = file_name + num + ".cnf";
        std::string command = "minisat " + dimacs_file_name;
        int result = system(command.c_str());

        std::string custom_msg = "";
        switch(result/256) {
            case 0: custom_msg = "Undefined!"; break;
            case 10: custom_msg = "Formulas f" + num + " and g" + num + " are NOT equivalent!"; break;
            case 20: custom_msg = "Formulas f" + num + " and g" + num + " are equivalent!"; break;
        }

        std::cout << custom_msg << std::endl << std::endl;
    }

    return 0;
}