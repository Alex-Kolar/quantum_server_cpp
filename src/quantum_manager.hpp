//
//  quantum_manager.hpp
//  server
//
//  Created by XIAOLIANG WU on 4/12/21.
//

#ifndef quantum_manager_hpp
#define quantum_manager_hpp

#include <stdio.h>
#include <vector>
#include <complex>
#include <map>
#include <shared_mutex>
#include <mutex>
#include <Eigen/Dense>
#include "qpp/qpp.h"

#include "circuit.hpp"
#include "utils.hpp"

#define CACHE_SIZE 1024

using namespace std;

class State{
public:
    vector<string> keys;
    Eigen::VectorXcd state;
    string serialization(){
        json j;
        j["keys"] = keys;
        vector<double> complex_vect;
        complex<double> c;
        for (u_int i = 0; i<state.size(); ++i){
            c = state(i);
            complex_vect.push_back(c.real());
            complex_vect.push_back(c.imag());
        }
        j["state"] = complex_vect;
        return j.dump();
    }
    State() {}
    State(Eigen::VectorXcd s, vector<string> k){
        assert (k.size() > 0);
        state = s;
        keys = k;
    }
};


class QuantumManager{
public:
    map<string, State*> states;
    shared_mutex map_lock;

    typedef tuple<Eigen::VectorXcd, vector<u_int>> key_type;
    typedef tuple<vector<double>, vector<qpp::cmat>> value_type;
    LRUCache<key_type, value_type*> measure_cache = LRUCache<key_type, value_type*>(CACHE_SIZE);

    State* get(string key){
        shared_lock lock(map_lock);
        return states[key];
    }
    void set(vector<string> ks, vector<double> amplitudes){
        unique_lock lock(map_lock);
        const unsigned long size = amplitudes.size() / 2;
        Eigen::VectorXcd complex_amp(size);

        for (int i=0; i < amplitudes.size(); i+=2) {
            complex<double> complex_num (amplitudes[i], amplitudes[i+1]);
            complex_amp(i / 2) = complex_num;
        }

        State* s = new State(complex_amp, ks);
        for (string k: ks){
            states[k] = s;
        }
    }
    void set(vector<string> ks, Eigen::VectorXcd amplitudes) {
        unique_lock lock(map_lock);
        State* s = new State(amplitudes, ks);
        for (string k: ks) {
            states[k] = s;
        }
    }
    map<string, int> run_circuit(Circuit*, vector<string>, float);

    bool exist(string key){
        shared_lock lock(map_lock);
        return states.find(key) != states.end();
    }

private:
    pair<Eigen::VectorXcd, vector<string>> prepare_state(vector<string>*);
    Eigen::VectorXcd vector_kron(Eigen::VectorXcd* first, Eigen::VectorXcd*);
    map<string, int> measure_helper(Eigen::VectorXcd, vector<u_int>, vector<string>, float);
};

//class QuantumManagerKet : public QuantumManager{
//public:
//    QuantumManagerKet();
//    void set(uint[], vector<complex<double>>);
//};

#endif /* quantum_manager_hpp */
