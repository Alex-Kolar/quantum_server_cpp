//
//  circuit.cpp
//  server
//
//  Created by XIAOLIANG WU on 4/12/21.
//

#include "circuit.hpp"
#include "nlohmann/json.hpp"
#include <vector>

using json = nlohmann::json;


Circuit::Circuit(json s_json){
    size = s_json["size"];
    for (auto gate: s_json["gates"]){
        std::string name = gate["name"];
        std::vector<int> indices = gate["indices"];
        std::pair<std::string, std::vector<int>> element = {name, indices};
        gates.push_back(element);
    }
    
    for (auto m_q: s_json["measured_qubits"]){
        measured_qubits.push_back(m_q);
    }
}

std::vector<std::vector<std::complex<double>>> Circuit::get_unitary_matrix(){
    // TODO
    std::vector<std::vector<std::complex<double>>> res;
    return res;
}
