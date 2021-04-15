//
//  circuit.cpp
//  server
//
//  Created by XIAOLIANG WU on 4/12/21.
//

#include "circuit.hpp"
#include "nlohmann/json.hpp"
#include <vector>
#include "qpp/qpp.h"
#include <Eigen/Dense>
#include <iostream>

using json = nlohmann::json;


Circuit::Circuit(json s_json){
    size = s_json["size"];
    for (auto gate: s_json["gates"]) {
        std::string name = gate["name"];
        std::vector<u_int> indices = gate["indices"];
        std::pair<std::string, std::vector<u_int>> element = {name, indices};
        gates.push_back(element);
    }
    
    for (auto m_q: s_json["measured_qubits"]){
        measured_qubits.push_back(m_q);
    }
}

//qpp::QCircuit Circuit::get_qcircuit() {
//    // TODO
//    using namespace qpp;
//
//    QCircuit qc{size, 0};
//
//    for (auto i: gates) {
//        std::string gate = i.first;
//        std::vector<u_int> indices = i.second;
//
//        std::cout << gate << std::endl;
//
//        if (gate == "h") {
//            qc.gate(gt.H, indices[0]);
//        } else if (gate == "x") {
//            qc.gate(gt.X, indices[0]);
//        } else if (gate == "y") {
//            qc.gate(gt.Y, indices[0]);
//        } else if (gate == "z") {
//            qc.gate(gt.Z, indices[0]);
//        } else if (gate == "cx") {
//            qc.CTRL(gt.X, indices[0], indices[1]);
//        } else if (gate == "ccx") {
//            throw std::logic_error("ccx gate not yet implemented");
//        } else if (gate == "swap") {
//            qc.gate(gt.SWAP, indices[0], indices[1]);
//        } else if (gate == "t") {
//            throw std::logic_error("t gate not yet implemented");
//        } else if (gate == "s") {
//            throw std::logic_error("s gate not yet implemented");
//        } else {
//            throw std::invalid_argument("Unknown gate " + gate);
//        }
//    }
//
//    return qc;
//}
