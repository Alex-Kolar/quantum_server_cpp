//
// Created by Alex Kolar on 4/21/21.
//
#include <map>
#include <vector>
#include <string>
#include "../src/qpp/qpp.h"

#include "../src/utils.hpp"

int main() {
    /// basic functionality

    LRUCache<int, double*> cache(3);

    auto* one_ptr = new double;
    *one_ptr = 1.0;
    auto* two_ptr = new double;
    *two_ptr = 2.0;
    auto* three_ptr = new double;
    *three_ptr = 3.0;
    auto* four_ptr = new double;
    *four_ptr = 4.0;

    cache.put(1, one_ptr);
    cache.put(2, two_ptr);

    cout << "Value in cache at 1: " << *cache.get(1) << endl;
    cout << "Value in cache at 2: " << *cache.get(2) << endl;
    if (!cache.get(3))
        cout << "Nothing stored at 3 yet." << endl;

    cache.put(3, three_ptr);
    cache.put(4, four_ptr);
    if (!cache.get(1))
        cout << "Value cached at 1 has been overwritten." << endl;

    /// with vectors

    LRUCache<Eigen::VectorXcd, map<string, int>*> cache_vector(3);

    Eigen::VectorXcd key(2);
    key(0) = 1;
    key(1) = 0;

    auto* value = new map<string, int>;
    (*value)["1"] = 1;

    cache_vector.put(key, value);
    if (cache_vector.get(key))
        cout << "Successfully stored vector." << endl;

    /// with tuple from measure function

    typedef tuple<Eigen::VectorXcd, vector<u_int>> key_type;
    typedef tuple<vector<double>, vector<qpp::cmat>> value_type;
    LRUCache<key_type, value_type*> cache_tuple(3);

    // define tuple inputs
    Eigen::VectorXcd tuple1(2);
    tuple1(0) = 1;
    tuple1(1) = 0;
    vector<u_int> tuple2;
    tuple2.push_back(0);

    key_type test_key (tuple1, tuple2);

    // define output
    auto* test_value = new value_type;

    // test cache
    cache_tuple.put(test_key, test_value);
    if (cache_tuple.get(test_key))
        cout << "Successfully stored tuple." << endl;
}
