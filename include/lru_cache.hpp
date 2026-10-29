#pragma once

#include <list>
#include <unordered_map>
#include <string>
#include <optional>
#include <mutex>

namespace spell {

template <typename Key, typename Value>
class LRUCache {
private:
    size_t capacity_;
    mutable size_t hits_{0};
    mutable size_t misses_{0};

    using ListIterator = typename std::list<std::pair<Key, Value>>::iterator;
    std::list<std::pair<Key, Value>> items_;
    std::unordered_map<Key, ListIterator> cache_map_;
    mutable std::mutex mutex_;

public:
    explicit LRUCache(size_t capacity = 1000) : capacity_(capacity) {}

    std::optional<Value> get(const Key& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            misses_++;
            return std::nullopt;
        }

        hits_++;
        // Move accessed item to front
        const_cast<std::list<std::pair<Key, Value>>&>(items_).splice(
            items_.begin(),
            const_cast<std::list<std::pair<Key, Value>>&>(items_),
            it->second
        );
        return it->second->second;
    }

    void put(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            it->second->second = value;
            items_.splice(items_.begin(), items_, it->second);
            return;
        }

        if (items_.size() >= capacity_) {
            auto last = items_.end();
            --last;
            cache_map_.erase(last->first);
            items_.pop_back();
        }

        items_.emplace_front(key, value);
        cache_map_[key] = items_.begin();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return items_.size();
    }

    size_t capacity() const { return capacity_; }
    size_t hits() const { return hits_; }
    size_t misses() const { return misses_; }

    double hit_rate() const {
        size_t total = hits_ + misses_;
        return total > 0 ? (static_cast<double>(hits_) / total) * 100.0 : 0.0;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        items_.clear();
        cache_map_.clear();
        hits_ = 0;
        misses_ = 0;
    }
};

} // namespace spell
