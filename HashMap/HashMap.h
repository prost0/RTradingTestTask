#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>
#include <algorithm>


const size_t deafaultBucketCount = 16;
const size_t DeafaultAllocMultiplier = 3;

template <typename Key, typename T, typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<void>>
class HashMap {
public:

	template <typename ContT, typename IterVal> struct HashMapIterator {

		bool operator==(const HashMapIterator &other) const {
		  return other.hm_ == hm_ && other.idx_ == idx_;
		}
		bool operator!=(const HashMapIterator &other) const {
		  return !(other == *this);
		}

		HashMapIterator &operator++() {
		  ++idx_;
		  advancePastEmpty();
		  return *this;
		}

		IterVal & operator*() const { return hm_->buckets_[idx_]; }
		IterVal * operator->() const { return &hm_->buckets_[idx_]; }

	private:
		explicit HashMapIterator(ContT *hm) : hm_(hm) { advancePastEmpty(); }
		explicit HashMapIterator(ContT *hm, std::size_t idx) : hm_(hm), idx_(idx) {}
		template <typename OtherContT, typename OtherIterVal>
		HashMapIterator(const HashMapIterator<OtherContT, OtherIterVal> &other)
		  : hm_(other.hm_), idx_(other.idx_) {}

		void advancePastEmpty() {
			while (idx_ < hm_->buckets_.size() &&
			       KeyEqual()(hm_->buckets_[idx_].first, hm_->emptyKey_)) {
			  ++idx_;
			}
		}

	  ContT *hm_ = nullptr;
	std::size_t idx_ = 0;
		friend ContT;
	};

public:
	HashMap(std::size_t bucketCount = deafaultBucketCount, Key emptyKey = Key()) : emptyKey_(emptyKey) {
		size_t pow2 = 1;
		while (pow2 < bucketCount) {
		  pow2 <<= 1;
		}
		buckets_.resize(pow2, std::make_pair(emptyKey_, T()));
	}

	HashMap(const HashMap &other, std::size_t bucketCount)
		  : HashMap(bucketCount, other.emptyKey_) {
		for (auto it = other.begin(); it != other.end(); ++it) {
			insert(*it);
		}
	}

	// Iterators
	HashMapIterator<HashMap, std::pair<Key, T>> begin() { return HashMapIterator<HashMap, std::pair<Key, T>>(this); }

	HashMapIterator<const HashMap, const std::pair<Key, T>> begin() const { return HashMapIterator<const HashMap, const std::pair<Key, T>>(this); }

	HashMapIterator<const HashMap, const std::pair<Key, T>> cbegin() const { return HashMapIterator<const HashMap, const std::pair<Key, T>>(this); }

	HashMapIterator<HashMap, std::pair<Key, T>> end() { return HashMapIterator<HashMap, std::pair<Key, T>>(this, buckets_.size()); }

	HashMapIterator<const HashMap, const std::pair<Key, T>> end() const { return HashMapIterator<const HashMap, const std::pair<Key, T>>(this, buckets_.size()); }

	HashMapIterator<const HashMap, const std::pair<Key, T>> cend() const { return HashMapIterator<const HashMap, const std::pair<Key, T>>(this, buckets_.size()); }

	// Capacity
	bool empty() const { return size() == 0; }

	std::size_t size() const { return size_; }

	std::size_t max_size() const { return std::numeric_limits<std::size_t>::max(); }

	// Modifiers
	void clear() {
		HashMap other(bucketCount(), emptyKey_);
		 swap(other);
	}

	std::pair<HashMapIterator<HashMap, std::pair<Key, T>>, bool> insert(const std::pair<Key, T> &value) {
		return emplaceImpl(value.first, value.second);
	}

	std::pair<HashMapIterator<HashMap, std::pair<Key, T>>, bool> insert(std::pair<Key, T> &&value) {
		return emplaceImpl(value.first, std::move(value.second));
	}

	template <typename... Args>
	std::pair<HashMapIterator<HashMap, std::pair<Key, T>>, bool> emplace(Args &&... args) {
		return emplaceImpl(std::forward<Args>(args)...);
	}

	void erase(HashMapIterator<HashMap, std::pair<Key, T>> it) { eraseImpl(it); }

	std::size_t erase(const Key &key) { return eraseImpl(key); }

	template <typename K> std::size_t erase(const K &x) { return eraseImpl(x); }

	void swap(HashMap &other) {
		std::swap(buckets_, other.buckets_);
		std::swap(size_, other.size_);
		std::swap(emptyKey_, other.emptyKey_);
	}

	// Lookup
	T &at(const Key &key) { return atImpl(key); }

	template <typename K> T &at(const K &x) { return atImpl(x); }

	const T &at(const Key &key) const { return atImpl(key); }

	template <typename K> const T &at(const K &x) const {
		return atImpl(x);
	}

	T &operator[](const Key &key) {
		return emplaceImpl(key).first->second;
	}

	std::size_t count(const Key &key) const { return count_impl(key); }

	template <typename K> std::size_t count(const K &x) const {
		return count_impl(x);
	}

	HashMapIterator<HashMap, std::pair<Key, T>> find(const Key &key) { return findImpl(key); }

	template <typename K> HashMapIterator<HashMap, std::pair<Key, T>> find(const K &x) { return findImpl(x); }

	HashMapIterator<const HashMap, const std::pair<Key, T>> find(const Key &key) const { return findImpl(key); }

	template <typename K> HashMapIterator<const HashMap, const std::pair<Key, T>> find(const K &x) const {
		return findImpl(x);
	}

	// Bucket interface
	std::size_t bucketCount() const noexcept { return buckets_.size(); }

	std::size_t max_bucketCount() const noexcept {
		return std::numeric_limits<std::size_t>::max();
	}

	// Hash policy
	void rehash(std::size_t count) {
		count = std::max(count, size() * 2);
		HashMap other(*this, count);
		swap(other);
	}

	void reserve(std::size_t count) {
		if (count * DeafaultAllocMultiplier > buckets_.size()) {
			rehash(count * DeafaultAllocMultiplier);
		}
	}

	// Observers
	Hash hash_function() const { return Hash(); }

	KeyEqual key_eq() const { return KeyEqual(); }

private:
	template <typename K, typename... Args>
	std::pair<HashMapIterator<HashMap, std::pair<Key, T>>, bool> emplaceImpl(const K &key, Args &&... args) {
		assert(!KeyEqual()(emptyKey_, key) && "Couldn't emplace empty key");
		reserve(size_ + 1);
		for (size_t idx = keyToIdx(key);; idx = probeNext(idx)) {
			if (KeyEqual()(buckets_[idx].first, emptyKey_)) {
				buckets_[idx].second = T(std::forward<Args>(args)...);
				buckets_[idx].first = key;
				++size_;
				return {HashMapIterator<HashMap, std::pair<Key, T>>(this, idx), true};
			} else if (KeyEqual()(buckets_[idx].first, key)) {
				return {HashMapIterator<HashMap, std::pair<Key, T>>(this, idx), false};
			}
		}
	}

	void eraseImpl(HashMapIterator<HashMap, std::pair<Key, T>> it) {
		size_t bucket = it.idx_;
		for (size_t idx = probeNext(bucket);; idx = probeNext(idx)) {
			if (KeyEqual()(buckets_[idx].first, emptyKey_)) {
				 buckets_[bucket].first = emptyKey_;
				 --size_;
				 return;
			}
			size_t ideal = keyToIdx(buckets_[idx].first);
			if (diff(bucket, ideal) < diff(idx, ideal)) {
				// swap, bucket is closer to ideal than idx
				buckets_[bucket] = buckets_[idx];
				bucket = idx;
			}
		}
	}

	template <typename K> std::size_t eraseImpl(const K &key) {
		auto it = findImpl(key);
		if (it != end()) {
			eraseImpl(it);
			return 1;
		}
		return 0;
	}

	template <typename K> T &atImpl(const K &key) {
		HashMapIterator<HashMap, std::pair<Key, T>> it = findImpl(key);
		if (it != end()) {
		  return it->second;
		}
		throw std::out_of_range("HashMap::at");
	}

	template <typename K> const T &atImpl(const K &key) const {
	  return const_cast<HashMap *>(this)->atImpl(key);
	}

	template <typename K> int64_t count_impl(const K &key) const {
	  return findImpl(key) == end() ? 0 : 1;
	}

	template <typename K> HashMapIterator<HashMap, std::pair<Key, T>> findImpl(const K &key) {
		assert(!KeyEqual()(emptyKey_, key) && "empty key shouldn't be used");
		for (size_t idx = keyToIdx(key);; idx = probeNext(idx)) {
			if (KeyEqual()(buckets_[idx].first, key)) {
				return HashMapIterator<HashMap, std::pair<Key, T>>(this, idx);
			}
			if (KeyEqual()(buckets_[idx].first, emptyKey_)) {
				return end();
			}
		}
	}

	template <typename K> HashMapIterator<const HashMap, const std::pair<Key, T>> findImpl(const K &key) const {
	  return const_cast<HashMap *>(this)->findImpl(key);
	}

	template <typename K> size_t keyToIdx(const K &key) const {
	  const size_t mask = buckets_.size() - 1;
	  return Hash()(key) & mask;
	}

	size_t probeNext(size_t idx) const {
	  const size_t mask = buckets_.size() - 1;
	  return (idx + 1) & mask;
	}

	size_t diff(size_t a, size_t b) const {
	  const size_t mask = buckets_.size() - 1;
	  return (buckets_.size() + (a - b)) & mask;
	}

private:
	  Key emptyKey_;
	  std::vector<std::pair<Key, T>> buckets_;
	  size_t size_ = 0;
	};
