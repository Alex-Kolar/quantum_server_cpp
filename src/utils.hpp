//
//  utils.hpp
//  server
//
//  Created by XIAOLIANG WU on 4/11/21.
//
//  Portions of LRU cache code from Tim Day's "LRU cache implementation in C++"
//  (https://timday.bitbucket.io/lru.html#x1-8007r1) with copyright info below
//
//  Copyright (c) 2010-2011, Tim Day <timday@timday.com>
//
//  Permission to use, copy, modify, and/or distribute this software for any
//  purpose with or without fee is hereby granted, provided that the above
//  copyright notice and this permission notice appear in all copies.
//
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
//  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
//  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
//  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#ifndef utils_hpp
#define utils_hpp

#include <stdio.h>
#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include <Eigen/Dense>

using namespace std;

void int_to_chars(u_long n, char* res);
uint chars_to_int(char* raw_data);
void send_msg_with_length(int socket, string message);
string recv_msg_with_length(int socket);
int rand_int(int low, int high);

// custom hash definitions for map: used in run_circuit
template <typename T>
void hash_accumulate(T input, size_t* seed) {
    (*seed) = std::hash<T>()(input) + 0x9e3779b9 + ((*seed) << 6) + ((*seed) >> 2);
}

namespace std {
    template <>
    struct hash<complex<double>> {
        size_t operator()(complex<double> const& comp) const {
            size_t seed = 0;
            hash_accumulate<double>(comp.real(), &seed);
            hash_accumulate<double>(comp.imag(), &seed);
            return seed;
        };
    };
    template <>
    struct hash<Eigen::VectorXcd> {
        size_t operator()(Eigen::VectorXcd const& matrix) const {
            size_t seed = 0;
            for (size_t i = 0; i < matrix.size(); ++i) {
                auto elem = *(matrix.data() + i);
                hash_accumulate<Eigen::VectorXcd::Scalar>(elem, &seed);
            }
            return seed;
        };
    };
    template <typename T>
    struct hash<vector<T>> {
        size_t operator()(vector<T> const& vect) const {
            size_t seed = 0;
            for (size_t i = 0; i < vect.size(); ++i) {
                T elem = vect[i];
                hash_accumulate<T>(elem, &seed);
            }
            return seed;
        }
    };
    template <>
    struct hash<tuple<Eigen::VectorXcd, vector<u_int>>> {
        size_t operator()(tuple<Eigen::VectorXcd, vector<u_int>> const& args) const {
            Eigen::VectorXcd first; vector<u_int> second;
            tie(first, second) = args;
            size_t seed = 0;
            hash_accumulate<Eigen::VectorXcd>(first, &seed);
            hash_accumulate<vector<u_int>>(second, &seed);
            return seed;
        }
    };
    template <>
    struct hash<tuple<Eigen::VectorXcd, string, vector<u_int>>> {
        size_t operator()(tuple<Eigen::VectorXcd, string, vector<u_int>> const& args) const {
            Eigen::VectorXcd first; string second; vector<u_int> third;
            tie(first, second, third) = args;
            size_t seed = 0;
            hash_accumulate<Eigen::VectorXcd>(first, &seed);
            hash_accumulate<string>(second, &seed);
            hash_accumulate<vector<u_int>>(third, &seed);
            return seed;
        }
    };
}

template <typename K, typename V> class LRUCache{
public:
    shared_mutex cache_lock;
    LRUCache(int maxsize){
        size = maxsize;
    }
    ~LRUCache() {
        for (auto k: key_list)
            delete cache[k].first;
    }
    V get(K);
    void put(K, V);
private:
    int size;
    list<K> key_list;
    unordered_map<K, pair<V, typename list<K>::iterator>> cache;
};

#include "utils.tpp"

#endif /* utils_hpp */
