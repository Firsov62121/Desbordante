#pragma once

#include <unordered_map>
#include <queue>
#include <cstddef>
#include <stdexcept>
#include <shared_mutex>
#include <mutex>
#include <boost/compute/detail/lru_cache.hpp>

namespace algos::fastod {

template <typename K, typename V>
class CacheWithLimit {
private:
    //std::unordered_map<K, V> entries_;
    //std::queue<K> keys_in_order_;
    boost::compute::detail::lru_cache<K, V> data_;
    const std::size_t max_size_;

public:
    //explicit CacheWithLimit(std::size_t max_size) : max_size_(max_size) {};
    explicit CacheWithLimit(std::size_t max_size) : data_(boost::compute::detail::lru_cache<K, V>(max_size)), max_size_(max_size) {};
    
    bool Contains(const K& key) {
        //return entries_.count(key) != 0;
        return data_.contains(key);
    }
    const V& Get(const K& key) {
        //return entries_.at(key);
        return data_.get(key).get();
    }
    void Set(const K& key, const V& value) {
        // if (Contains(key)) {
        //     throw std::logic_error("Updaing a cache entry is not supported");
        // }

        //

        // if (keys_in_order_.size() >= max_size_) {
        //     entries_.erase(keys_in_order_.front());
        //     keys_in_order_.pop();
        // }

        // keys_in_order_.push(key);
        // entries_.emplace(key, value);

        data_.insert(key, value);
    }
};

} // namespace algos::fastod;
