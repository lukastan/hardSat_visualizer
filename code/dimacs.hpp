#include "normalform.hpp"

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