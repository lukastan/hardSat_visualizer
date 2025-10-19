#include <iostream>
#include <variant>
#include <memory>
#include <map>
#include <string>
#include <sstream>
#include <fstream>

#include "json.hpp"

using json = nlohmann::json;

struct False;
struct True;
struct Atom;
struct Not;
struct Binary;

using Formula = std::variant<False, True, Atom, Not, Binary>;
using FormulaPtr = std::shared_ptr<Formula>;

struct False {};
struct True {};
struct Atom { std::string name; };
struct Not { FormulaPtr subformula; };
struct Binary {
    enum Type { And, Or, Impl, Eq } type;
    FormulaPtr left, right;
};

FormulaPtr ptr(const Formula& f) { return std::make_shared<Formula>(f); }

template<typename T>
bool is(const FormulaPtr& f) { return std::holds_alternative<T>(*f); }

template<typename T>
T as(const FormulaPtr& f) { return std::get<T>(*f); }

struct JsonGraph {
    int nextId = 0;
    std::map<std::string, int> atomIds;
    std::map<int, int> dist;
    json j = {{"nodes", json::array()}, {"edges", json::array()} };
    std::map<int, std::vector<int>> neighbours;

    void calc_dist_rec(int id, int counter) {
        for(auto neigh : neighbours[id]) {
            if(j["nodes"][neigh]["dist"] == -1) {
                j["nodes"][neigh]["dist"] = counter + 1;
                calc_dist_rec(neigh, counter + 1);
            }
        }
    }

    void calc_dist() {
        for(auto &edge : j["edges"]) {
            int src = edge[0];
            int dest = edge[1];
            neighbours[src].push_back(dest);
        }
        for(auto &node : j["nodes"]) {
            if(node["type"] == "input") {
                node["dist"] = 0;
                calc_dist_rec(node["id"], 0);
            }
        }
    }

    int add_node(const std::string &label, const std::string &type) {
        int id = nextId++;
        json node = {{"id", id}, {"label", label}, {"type", type}, {"dist", -1}};
        j["nodes"].push_back(node);
        return id;
    }

    int from_formula(const FormulaPtr &f) {
        if (is<Atom>(f)) {
            std::string name = as<Atom>(f).name;
            if (atomIds.count(name)) return atomIds[name];
            int id = add_node(name, "input");
            atomIds[name] = id;
            return id;
        }
        if (is<Not>(f)) {
            int c = from_formula(as<Not>(f).subformula);
            int id = add_node("NOT", "gate");
            j["edges"].push_back({c, id});
            return id;
        }
        if (is<Binary>(f)) {
            auto b = as<Binary>(f);
            int L = from_formula(b.left);
            int R = from_formula(b.right);
            std::string label;
            switch(b.type) {
                case Binary::And: label="AND"; break;
                case Binary::Or:  label="OR";  break;
                case Binary::Impl: label="IMPL"; break;
                case Binary::Eq:  label="EQ";  break;
            }
            int id = add_node(label, "gate");
            j["edges"].push_back({L, id});
            j["edges"].push_back({R, id});
            return id;
        }
        if (is<True>(f)) return add_node("T", "input");
        if (is<False>(f)) return add_node("F", "input");
        return -1;
    }

    json to_json(const FormulaPtr &f) {
        int root = from_formula(f);
        calc_dist();

        return j;
    }
};