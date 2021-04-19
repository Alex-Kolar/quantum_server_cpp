//
// Created by Alex Kolar on 4/14/21.
//
#include <iostream>
#include "../src/circuit.hpp"
#include "../src/quantum_manager.hpp"

int main() {
    using namespace qpp;

    json input = json();
    std::vector<u_int> indices(1, 0);
    input["size"] = 1;
    input["gates"] = {{{"name", "h"}, {"indices", indices}}};
    Circuit* c = new Circuit(input);

    cout << "input json: " << input << endl;

    // create test manager
    vector<string> keys = {"1"};
    vector<double> amplitudes = {1, 0, 0, 0};
    QuantumManager qm = QuantumManager();
    qm.set(keys, amplitudes);

    cout << "input state:" << endl;
    cout << disp(qm.get("1").state) << endl;

    // run circuit and observe
    qm.run_circuit(c, keys, 0.5);
    auto new_state = qm.get("1");

    cout << "new state:" << endl;
    cout << disp(new_state.state) << endl;

    Eigen::VectorXcd test_state(2);
    test_state(0) = complex<double>(1, 0);
    test_state(1) = complex<double>(0, 0);
}