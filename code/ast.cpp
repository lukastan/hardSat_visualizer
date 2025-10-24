#include "jsonGraph.hpp"
#include "dimacs.hpp"

#include <bits/stdc++.h>
#include <algorithm>

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