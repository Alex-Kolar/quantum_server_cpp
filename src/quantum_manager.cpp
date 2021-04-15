//
//  quantum_manager.cpp
//  server
//
//  Created by XIAOLIANG WU on 4/12/21.
//

#include <iostream>
#include "quantum_manager.hpp"
#include "circuit.hpp"
#include <map>
#include <vector>
#include "qpp/qpp.h"

map<string, int> QuantumManager::run_circuit(Circuit* circuit, vector<string> keys, float meas_samp){
    using namespace qpp;

    // prepare circuit
    auto prepared = prepare_state(&keys);
    auto state = prepared.first;
    auto all_keys = prepared.second;

    // run circuit
    for (auto i: circuit->get_gates()) {
        string gate = i.first;
        vector<u_int> indices = i.second;

        if (gate == "h") {
            apply(state, gt.H, {indices[0]}, 2);
        } else if (gate == "x") {
            apply(state, gt.X, {indices[0]}, 2);
        } else if (gate == "y") {
            apply(state, gt.Y, {indices[0]}, 2);
        } else if (gate == "z") {
            apply(state, gt.Z, {indices[0]}, 2);
        } else if (gate == "cx") {
            applyCTRL(state, gt.X, {indices[0]}, {indices[1]});
        } else if (gate == "swap") {
            apply(state, gt.SWAP, {indices[0], indices[1]}, 4);
        }
    }

    std::cout << disp(state) << std::endl;

    map<string, int> res;

    if (circuit->get_measured().empty()) {
        set(all_keys, state);
        return res;
    } else {
        throw std::logic_error("circuit measurement not yet implemented");
    }
}

std::pair<Eigen::VectorXcd, std::vector<string>> QuantumManager::prepare_state(std::vector<string>* keys) {
    vector<Eigen::VectorXcd> old_states;
    vector<string> all_keys;

    // get all required states
    for (auto key: *keys) {
        if (find(all_keys.begin(), all_keys.end(), key) == all_keys.end()) {
            auto state = get(key);
            old_states.push_back(state.state);
            all_keys.insert(all_keys.end(), state.keys.begin(), state.keys.end());
        }
    }

    // compound states
    Eigen::VectorXcd new_state;
    new_state << complex<double>(1, 0);
    for (auto state: old_states) {
        new_state = qpp::kron(new_state, state);
    }

    // TODO: swap qubits if necessary

    pair<Eigen::VectorXcd, vector<string>> res = {new_state, all_keys};
    return res;
}
