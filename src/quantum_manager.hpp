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
#include "circuit.hpp"
#include <Eigen/Dense>

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
        state = s;
        keys = k;
    }
};

class QuantumManager{
public:
    map<string, State> states;

    State get(string key){
        return states[key];
    }
    void set(vector<string> ks, vector<double> amplitudes){
        const unsigned long size = amplitudes.size() / 2;
        Eigen::VectorXcd complex_amp(size);

        for (int i=0; i < amplitudes.size(); i+=2) {
            complex<double> complex_num (amplitudes[i], amplitudes[i+1]);
            complex_amp(i / 2) = complex_num;
        }

        State s = State(complex_amp, ks);
        for (string k: ks){
            states[k] = s;
        }
    }
    void set(vector<string> ks, Eigen::VectorXcd amplitudes) {
        State s = State(amplitudes, ks);
        for (string k: ks) {
            states[k] = s;
        }
    }
    map<string, int> run_circuit(Circuit*, vector<string>, float);
private:
    std::pair<Eigen::VectorXcd, std::vector<string>> prepare_state(std::vector<string>* keys);
    Eigen::VectorXcd vector_kron(Eigen::VectorXcd* first, Eigen::VectorXcd* second);
};

//class QuantumManagerKet : public QuantumManager{
//public:
//    QuantumManagerKet();
//    void set(uint[], vector<complex<double>>);
//};

#endif /* quantum_manager_hpp */
