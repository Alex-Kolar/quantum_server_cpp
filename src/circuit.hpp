//
//  circuit.hpp
//  server
//
//  Created by XIAOLIANG WU on 4/12/21.
//

#ifndef circuit_hpp
#define circuit_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include <complex>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Circuit{
    int size;
    std::vector<std::pair<std::string, std::vector<int>>> gates;
    std::vector<int> measured_qubits;
public:
    Circuit(json);
    std::vector<std::vector<std::complex<double>>> get_unitary_matrix();
};

#endif /* circuit_hpp */
