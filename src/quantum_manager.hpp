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

using namespace std;

class State{
public:
    vector<string> keys;
    vector<complex<double>> state;
    string serialization(){
        json j;
        j["keys"] = keys;
        vector<double> complex_vect;
        for (complex<double> c: state){
            complex_vect.push_back(c.real());
            complex_vect.push_back(c.imag());
        }
        j["state"] = complex_vect;
        return j.dump();
    }
    State() {}
    State(vector<complex<double>> s, vector<string> k){
        state = s;
        keys = k;
    }
};

class QuantumManager{
    map<string, State> states;
public:
    State get(string key){
        return states[key];
    }
    void set(vector<string> ks, vector<double> amplitudes){
        vector<complex<double>> real_amp;
        for (int i=0; i < amplitudes.size(); i+=2) {
            complex<double> complex_num = amplitudes[i] + i * amplitudes[i+1];
            real_amp.push_back(complex_num);
        }
        State s = State(real_amp, ks);
        for (string k:ks){
            states[k] = s;
        }
    }
    map<string, int> run_circuit(Circuit*, vector<string>, float);
};

//class QuantumManagerKet : public QuantumManager{
//public:
//    QuantumManagerKet();
//    void set(uint[], vector<complex<double>>);
//};

#endif /* quantum_manager_hpp */
