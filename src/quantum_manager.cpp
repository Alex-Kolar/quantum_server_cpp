//
//  quantum_manager.cpp
//  server
//
//  Created by XIAOLIANG WU on 4/12/21.
//

#include <iostream>
#include <map>
#include <vector>
#include "qpp/qpp.h"

#include "quantum_manager.hpp"
#include "circuit.hpp"
#include "utils.hpp"

#define CACHE_SIZE 1024

using namespace qpp;

// global caches
LRUCache<measure_key_type, measure_value_type*> measure_cache =
        LRUCache<measure_key_type, measure_value_type*>(CACHE_SIZE);
LRUCache<apply_key_type, apply_value_type*> h_cache =
        LRUCache<apply_key_type, apply_value_type*>(CACHE_SIZE);
LRUCache<apply_key_type, apply_value_type*> x_cache =
        LRUCache<apply_key_type, apply_value_type*>(CACHE_SIZE);
LRUCache<apply_key_type, apply_value_type*> y_cache =
        LRUCache<apply_key_type, apply_value_type*>(CACHE_SIZE);
LRUCache<apply_key_type, apply_value_type*> z_cache =
        LRUCache<apply_key_type, apply_value_type*>(CACHE_SIZE);
LRUCache<apply_key_type, apply_value_type*> ctrlx_cache =
        LRUCache<apply_key_type, apply_value_type*>(CACHE_SIZE);
LRUCache<apply_key_type, apply_value_type*> swap_cache =
        LRUCache<apply_key_type, apply_value_type*>(CACHE_SIZE);

map<string, int> QuantumManager::run_circuit(Circuit* circuit, vector<string> keys, float meas_samp){
    // prepare circuit
    auto prepared = prepare_state(&keys);
    auto state = prepared.first;
    auto all_keys = prepared.second;

    // run circuit
    for (auto i: circuit->get_gates()) {
        string gate = i.first;
        vector<u_int> indices = i.second;
        state = apply_wrapper(state, gate, indices);
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
            old_states.push_back(state->state);
            all_keys.insert(all_keys.end(), state->keys.begin(), state->keys.end());
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
    vector<double> probs;
    vector<cmat> resultant_states;

    // check cache for result
    measure_key_type key = make_tuple(state, indices);
    measure_value_type* value_ptr = measure_cache.get(key);

    if (value_ptr) {
        probs = std::get<0>(*value_ptr);
        resultant_states = std::get<1>(*value_ptr);

    } else {
        // convert input indices to idx
        vector<idx> indices_idx(num_qubits_meas);
        for (int i = 0; i < num_qubits_meas; i++) {
            indices_idx[i] = (idx) indices[i];
        }

        // obtain measurement data using qpp
        auto meas_data = measure(state, gt.Id(1 << num_qubits_meas), indices_idx);
        probs = std::get<PROB>(meas_data);
        resultant_states = std::get<ST>(meas_data);

        // store in cache
        value_ptr = new measure_value_type;
        *value_ptr = make_pair(probs, resultant_states);
        measure_cache.put(key, value_ptr);
    }

    // determine measurement result using random sample
    double cum_sum = 0;
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

Eigen::VectorXcd QuantumManager::apply_wrapper(Eigen::VectorXcd state, string gate, vector<u_int> indices) {
    Eigen::VectorXcd output_state(state.rows());
    apply_key_type key = make_tuple(state, gate, indices);
    apply_value_type* value_ptr;

    if (gate == "h") {
        value_ptr = h_cache.get(key);
        if (value_ptr)
            output_state = *value_ptr;
        else {
            output_state = apply(state, gt.H, {indices[0]});
            value_ptr = new apply_value_type;
            *value_ptr = output_state;
            h_cache.put(key, value_ptr);
        }

    } else if (gate == "x") {
        value_ptr = x_cache.get(key);
        if (value_ptr)
            output_state = *value_ptr;
        else {
            output_state = apply(state, gt.X, {indices[0]});
            value_ptr = new apply_value_type;
            *value_ptr = output_state;
            x_cache.put(key, value_ptr);
        }

    } else if (gate == "y") {
        value_ptr = y_cache.get(key);
        if (value_ptr)
            output_state = *value_ptr;
        else {
            output_state = apply(state, gt.Y, {indices[0]});
            value_ptr = new apply_value_type;
            *value_ptr = output_state;
            y_cache.put(key, value_ptr);
        }

    } else if (gate == "z") {
        value_ptr = z_cache.get(key);
        if (value_ptr)
            output_state = *value_ptr;
        else {
            output_state = apply(state, gt.Z, {indices[0]});
            value_ptr = new apply_value_type;
            *value_ptr = output_state;
            z_cache.put(key, value_ptr);
        }

    } else if (gate == "cx") {
        value_ptr = ctrlx_cache.get(key);
        if (value_ptr)
            output_state = *value_ptr;
        else {
            output_state = applyCTRL(state, gt.X, {indices[0]}, {indices[1]});
            value_ptr = new apply_value_type;
            *value_ptr = output_state;
            ctrlx_cache.put(key, value_ptr);
        }

    } else if (gate == "swap") {
        value_ptr = swap_cache.get(key);
        if (value_ptr)
            output_state = *value_ptr;
        else {
            output_state = apply(state, gt.SWAP, {indices[0], indices[1]});
            value_ptr = new apply_value_type;
            *value_ptr = output_state;
            swap_cache.put(key, value_ptr);
        }

    } else {
        throw std::invalid_argument("undefined gate " + gate);
    }

    return output_state;
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
