#include <iostream>
#include <variant>
#include <memory>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>

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
    int nextId = 1;
    json j = {{"nodes", json::array()}, {"edges", json::array()} };

    std::unordered_map<const Formula*, int> gateIds;
    std::unordered_map<std::string, int> inputIds;

    void add_node(const std::string &label, const std::string &type, int id, int dist) {
        json node = {{"id", id}, {"label", label}, {"type", type}, {"dist", dist}};
        j["nodes"].push_back(node);
    }

    int ensure_input(const std::string &name, int dist) {
        auto it = inputIds.find(name);
        if (it != inputIds.end())
            return it->second;

        int id = nextId++;
        inputIds[name] = id;
        add_node(name, "input", id, dist);
        return id;
    }

    int ensure_gate(const FormulaPtr &f, const std::string &label, int dist) {
        const Formula* key = f.get();
        auto it = gateIds.find(key);
        if (it != gateIds.end())
            return it->second;

        int id = nextId++;
        gateIds[key] = id;
        add_node(label, "gate", id, dist);
        return id;
    }

   void from_formula(const FormulaPtr &f, int parent_id, int dist) {
        if (!f) return;

        if (is<Atom>(f)) {
            std::string name = as<Atom>(f).name;
            int this_id = ensure_input(name, dist);
            if (parent_id != 0)
                j["edges"].push_back({this_id, parent_id});
        }
        else if (is<Not>(f)) {
            int this_id = ensure_gate(f, "NOT", dist);
            if (parent_id != 0)
                j["edges"].push_back({this_id, parent_id});
            from_formula(as<Not>(f).subformula, this_id, dist + 1);
        }
        else if (is<Binary>(f)) {
            auto b = as<Binary>(f);
            std::string label;
            switch(b.type) {
                case Binary::And: label="AND"; break;
                case Binary::Or:  label="OR";  break;
                case Binary::Impl: label="IMPL"; break;
                case Binary::Eq:  label="EQ";  break;
            }
            int this_id = ensure_gate(f, label, dist);
            if (parent_id != 0)
                j["edges"].push_back({this_id, parent_id});
            from_formula(b.left, this_id, dist + 1);
            from_formula(b.right, this_id, dist + 1);
        }
    }

    json to_json(const FormulaPtr &f) {
        gateIds.clear();
        inputIds.clear();;
        j["nodes"].clear();
        j["edges"].clear();
        nextId = 1;
        from_formula(f, 0, 0);
        int max_dist = 0;
        for(auto& node : j["nodes"])
            if(node["dist"] > max_dist)
                max_dist = node["dist"];
        max_dist++;
        for(auto& node : j["nodes"])
            if(node["type"] == "input")
                node["dist"] = max_dist;

        return j;
    }
};