#include <shared_mutex>
#include "qpp/qpp.h"

// type definitions for LRUCaches in quantum manager
typedef tuple<Eigen::VectorXcd, vector<u_int>> measure_key_type;
typedef tuple<vector<double>, vector<qpp::cmat>> measure_value_type;
typedef tuple<Eigen::VectorXcd, string, vector<u_int>> apply_key_type;
typedef Eigen::VectorXcd apply_value_type;

template <typename T>
void hash_accumulate(T input, size_t* seed) {
    (*seed) = std::hash<T>()(input) + 0x9e3779b9 + ((*seed) << 6) + ((*seed) >> 2);
}

// custom hash/equal definitions for map
// used for caching vectors/etc. in LRUCache application
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
    struct hash<measure_key_type> {
        size_t operator()(measure_key_type const& args) const {
            Eigen::VectorXcd first; vector<u_int> second;
            tie(first, second) = args;
            size_t seed = 0;
            hash_accumulate<Eigen::VectorXcd>(first, &seed);
            hash_accumulate<vector<u_int>>(second, &seed);
            return seed;
        }
    };
    template <>
    struct hash<apply_key_type> {
        size_t operator()(apply_key_type const& args) const {
            Eigen::VectorXcd first; string second; vector<u_int> third;
            tie(first, second, third) = args;
            size_t seed = 0;
            hash_accumulate<Eigen::VectorXcd>(first, &seed);
            hash_accumulate<string>(second, &seed);
            hash_accumulate<vector<u_int>>(third, &seed);
            return seed;
        }
    };

    template <>
    struct equal_to<measure_key_type> {
        bool operator() (const measure_key_type x, const measure_key_type y) const {
            if ((get<0>(x).rows() != get<0>(y).rows()) or (get<0>(x).cols() != get<0>(y).cols()))
                return false;
            return (get<0>(x) == get<0>(y)) and (get<1>(x) == get<1>(y));
        }
    };

    template <>
    struct equal_to<apply_key_type> {
        bool operator() (const apply_key_type x, const apply_key_type y) const {
            if ((get<0>(x).rows() != get<0>(y).rows()) or (get<0>(x).cols() != get<0>(y).cols()))
                return false;
            return (get<0>(x) == get<0>(y)) and (get<1>(x) == get<1>(y)) and (get<2>(x) == get<2>(y));
        }
    };
}

template<typename K, typename V>
void LRUCache<K, V>::put(K key, V value) {
    unique_lock(this->cache_lock);

    // mark key as most recently accessed and insert into cache
    auto it = key_list.insert(key_list.begin(), key);
    auto pair = make_pair(value, it);
    cache[key] = pair;

    // remove old keys if necessary
    if (key_list.size() > size) {
        K old_key = key_list.back();
        auto it_cache = cache.find(old_key);

        delete it_cache->second.first;

        cache.erase(it_cache);
        key_list.pop_back();
    }
}

template<typename K, typename V>
V LRUCache<K, V>::get(K input_key) {
    shared_lock(this->cache_lock);

    auto it = cache.find(input_key);

    // cache miss
    if (it == cache.end())
        return nullptr;

    // cache hit
    // update key as most recently accessed
    auto it_keys = it->second.second;
    key_list.splice(key_list.begin(), key_list, it_keys);

    // return value
    return it->second.first;
}
