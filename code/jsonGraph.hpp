#include "formula.hpp"
#include "json.hpp"

using json = nlohmann::json;

struct JsonGraph {
    int nextId = 1;
    json j = {{"nodes", json::array()}, {"edges", json::array()} };

    std::unordered_map<const Formula*, int> gateIds;
    std::unordered_map<std::string, int> inputIds;

    std::unordered_map<int, std::vector<bool>> truthVectors;

    bool evaluate(const FormulaPtr& f, Valuation& v, int vId) {
        if(is<False>(f))
            return false;
        if(is<True>(f))
            return true;
        if(is<Atom>(f)) {
            std::string name = as<Atom>(f).name;
            bool value = v[name];
            int nodeId = inputIds[name];
            truthVectors[nodeId][vId] = value;
            return value;
        }
        if(is<Not>(f)) {
            bool value = !evaluate(as<Not>(f).subformula, v, vId);
            int nodeId = gateIds[f.get()];
            truthVectors[nodeId][vId] = value;
            return value;
        }
        if(is<Binary>(f)) {
            bool evalL = evaluate(as<Binary>(f).left, v, vId);
            bool evalR = evaluate(as<Binary>(f).right, v, vId);
            bool value = false;
            switch(as<Binary>(f).type) {
                case Binary::And:  value = evalL && evalR;
                case Binary::Or:   value = evalL || evalR;
                case Binary::Impl: value = !evalL || evalR;
                case Binary::Eq:   value = evalL == evalR;
            }
            int nodeId = gateIds[f.get()];
            truthVectors[nodeId][vId] = value;
            return value;
        }
        return false;
    }

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

        AtomSet atoms;
        getAtoms(f, atoms);
        std::vector<Valuation> valuations = getValuations(atoms);

        int n = valuations.size();
        for(auto &[_, id] : gateIds)
            truthVectors[id] = std::vector<bool>(n, false);
        for(auto &[_, id] : inputIds)
            truthVectors[id] = std::vector<bool>(n, false);

        for(int i = 0; i < n; i++)
            evaluate(f, valuations[i], i);

        for(auto& node : j["nodes"]) {
            if(node["type"] == "input")
                node["dist"] = max_dist;
            node["valuation"] = truthVectors[node["id"]];
        }

        return j;
    }
};