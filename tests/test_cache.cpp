//
// Created by Alex Kolar on 4/21/21.
//
#include <map>
#include <vector>
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

    auto* map_value_ptr = new map<string, int>;
    (*map_value_ptr)["1"] = 1;

    cache_vector.put(key, map_value_ptr);
    if (cache_vector.get(key))
        cout << "Successfully stored vector key." << endl;

    /// with tuple from measure function

    LRUCache<key_type, measure_value_type*> cache_tuple(3);

    // define tuple inputs
    Eigen::VectorXcd tuple1(2);
    tuple1(0) = 1;
    tuple1(1) = 0;
    vector<u_int> tuple2;
    tuple2.push_back(0);

    key_type measure_key (tuple1, tuple2);

    // define output
    auto* measure_value_ptr = new measure_value_type;

    // test cache
    cache_tuple.put(measure_key, measure_value_ptr);
    if (cache_tuple.get(measure_key))
        cout << "Successfully stored tuple." << endl;

    /// with vectors as values

    LRUCache<string, Eigen::VectorXcd*> cache_vector_val(10);

    string str_key1 = "1";
    auto* vec_val1_ptr = new Eigen::VectorXcd(2);
    (*vec_val1_ptr)(0) = 1;
    (*vec_val1_ptr)(1) = 0;
    string str_key2 = "2";
    auto* vec_val2_ptr = new Eigen::VectorXcd(2);
    (*vec_val2_ptr)(0) = 1;
    (*vec_val2_ptr)(1) = 0;
    string str_key3 = "3";
    auto* vec_val3_ptr = new Eigen::VectorXcd(4);
    (*vec_val3_ptr)(0) = 1;
    (*vec_val3_ptr)(1) = 0;
    (*vec_val3_ptr)(2) = 0;
    (*vec_val3_ptr)(3) = 0;

    cache_vector_val.put(str_key1, vec_val1_ptr);
    cache_vector_val.put(str_key2, vec_val2_ptr);
    cache_vector_val.put(str_key3, vec_val3_ptr);
    if (cache_vector_val.get(str_key1))
        cout << "Successfully stored Eigen::vector." << endl;
}
