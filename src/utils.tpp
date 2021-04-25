#include <shared_mutex>

#include "utils.hpp"

template<typename K, typename V>
void LRUCache<K, V>::put(K key, V value) {
    unique_lock(this->cache_lock);

    // mark key as most recently accessed and insert into cache
    auto it = key_list.insert(key_list.begin(), key);
    cache[key] = make_pair(value, it);

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
