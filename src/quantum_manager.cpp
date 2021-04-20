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

using namespace qpp;

map<string, int> QuantumManager::run_circuit(Circuit* circuit, vector<string> keys, float meas_samp){
    // prepare circuit
    auto prepared = prepare_state(&keys);
    auto state = prepared.first;
    auto all_keys = prepared.second;

    // run circuit
    for (auto i: circuit->get_gates()) {
        string gate = i.first;
        vector<u_int> indices = i.second;

        if (gate == "h") {
            state = apply(state, gt.H, {indices[0]});
        } else if (gate == "x") {
            state = apply(state, gt.X, {indices[0]});
        } else if (gate == "y") {
            state = apply(state, gt.Y, {indices[0]});
        } else if (gate == "z") {
            state = apply(state, gt.Z, {indices[0]});
        } else if (gate == "cx") {
            state = applyCTRL(state, gt.X, {indices[0]}, {indices[1]});
        } else if (gate == "swap") {
            state = apply(state, gt.SWAP, {indices[0], indices[1]});
        } else {
            throw std::invalid_argument("undefined gate " + gate);
        }
    }

    auto meas_indices = circuit->get_measured();
    if (meas_indices.empty()) {
        set(all_keys, state);
        return map<string, int>();
    }
    return measure_helper(state, meas_indices, all_keys, meas_samp);
}

std::pair<Eigen::VectorXcd, std::vector<string>> QuantumManager::prepare_state(std::vector<string>* keys) {
    vector<Eigen::VectorXcd> old_states;
    vector<string> all_keys;

    // get all required states
    for (string key: *keys) {
        if (find(all_keys.begin(), all_keys.end(), key) == all_keys.end()) {
            auto state = get(key);
            old_states.push_back(state.state);
            all_keys.insert(all_keys.end(), state.keys.begin(), state.keys.end());
        }
    }

    // compound states
    Eigen::VectorXcd new_state(1);
    new_state(0) = complex<double>(1, 0);
    for (auto state: old_states) {
        new_state = vector_kron(&new_state, &state);
    }

    // swap qubits if necessary
    string proper_key;
    u_int j;
    auto it = all_keys.begin();
    for (u_int i = 0; i < keys->size(); i++) {
        if (all_keys[i] != (*keys)[i]) {
            proper_key = (*keys)[i];
            it = std::find(all_keys.begin(), all_keys.end(), proper_key);
            j = it - all_keys.begin(); // should always find proper_key in all_keys

            // perform swapping operation on state
            new_state = apply(new_state, gt.SWAP, {i, j});

            // swap keys
            all_keys[j] = all_keys[i];
            all_keys[i] = proper_key;
        }
    }

    pair<Eigen::VectorXcd, vector<string>> res = {new_state, all_keys};
    return res;
}

map<string, int> QuantumManager::measure_helper(Eigen::VectorXcd state,
                                                vector<u_int> indices,
                                                vector<string> all_keys,
                                                float samp) {
    auto num_qubits_meas = indices.size();

    // convert input indices to idx
    vector<idx> indices_idx(num_qubits_meas);
    for (int i = 0; i < num_qubits_meas; i++) {
        indices_idx[i] = (idx)indices[i];
    }

    // obtain measurement data using qpp
    auto meas_data = measure(state, gt.Id(1 << num_qubits_meas), indices_idx);
    vector<double> probs = std::get<PROB>(meas_data);
    vector<cmat> resultant_states = std::get<ST>(meas_data);

    // determine measurement result using random sample
    int cum_sum = 0;
    int res = 0;
    while (res < probs.size()) {
        cum_sum += probs[res];
        if (samp < cum_sum) {
            break;
        }
        res++;
    }

    // assign states
    map<string, int> output;
    auto start = all_keys.begin();
    u_int index;
    int res_bit;
    Eigen::VectorXcd state0(2);
    state0(0) = complex<double>(1,0);
    state0(1) = complex<double>(0,0);
    Eigen::VectorXcd state1(2);
    state1(0) = complex<double>(0,0);
    state1(1) = complex<double>(1,0);
    vector<Eigen::VectorXcd> output_states = {state0, state1};

    for (int i = num_qubits_meas-1; i >= 0; i--) {
        index = indices[i];
        res_bit = (res >> (num_qubits_meas-1-i)) & 1;
        set({all_keys[index]}, output_states[res_bit]);
        output[all_keys[index]] = res_bit;

        all_keys.erase(start + index);
    }

    if (!all_keys.empty())
        set(all_keys, resultant_states[res]);

    return output;
}

Eigen::VectorXcd QuantumManager::vector_kron(Eigen::VectorXcd* first, Eigen::VectorXcd* second) {
    long first_size = first->rows();
    long second_size = second->rows();
    long size = first_size * second_size;
    Eigen::VectorXcd out(size);

    for (int i = 0; i < size; i++) {
        out(i) = (*first)(i / second_size) * (*second)(i % second_size);
    }

    return out;
}
