// CityHash obtained from https://github.com/google/cityhash

// Copyright (c) 2011 Google, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// CityHash, by Geoff Pike and Jyrki Alakuijala
//
// http://code.google.com/p/cityhash/
//
// This file provides a few functions for hashing strings.  All of them are
// high-quality functions in the sense that they pass standard tests such
// as Austin Appleby's SMHasher.  They are also fast.
//
// For 64-bit x86 code, on short strings, we don't know of anything faster than
// CityHash64 that is of comparable quality.  We believe our nearest competitor
// is Murmur3.  For 64-bit x86 code, CityHash64 is an excellent choice for hash
// tables and most other hashing (excluding cryptography).
//
// For 64-bit x86 code, on long strings, the picture is more complicated.
// On many recent Intel CPUs, such as Nehalem, Westmere, Sandy Bridge, etc.,
// CityHashCrc128 appears to be faster than all competitors of comparable
// quality.  CityHash128 is also good but not quite as fast.  We believe our
// nearest competitor is Bob Jenkins' Spooky.  We don't have great data for
// other 64-bit CPUs, but for long strings we know that Spooky is slightly
// faster than CityHash on some relatively recent AMD x86-64 CPUs, for example.
// Note that CityHashCrc128 is declared in citycrc.h.
//
// For 32-bit x86 code, we don't know of anything faster than CityHash32 that
// is of comparable quality.  We believe our nearest competitor is Murmur3A.
// (On 64-bit CPUs, it is typically faster to use the other CityHash variants.)
//
// Functions in the CityHash family are not suitable for cryptography.
//
// Please see CityHash's README file for more details on our performance
// measurements and so on.
//
// WARNING: This code has been only lightly tested on big-endian platforms!
// It is known to work well on little-endian platforms that have a small penalty
// for unaligned reads, such as current Intel and AMD moderate-to-high-end CPUs.
// It should work on all 32-bit and 64-bit platforms that allow unaligned reads;
// bug reports are welcome.
//
// By the way, for some hash functions, given strings a and b, the hash
// of a+b is easily derived from the hashes of a and b.  This property
// doesn't hold for any hash functions in this file.

#ifndef CITY_HASH_H_
#define CITY_HASH_H_

#include <stdlib.h>  // for size_t.
#include <stdint.h>
#include <utility>
#include <string>
#include <cstring>
#include <vector>
#include <memory>
#include "utils/utils.h"

namespace GTLib2 {

typedef uint8_t uint8;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef std::pair<uint64, uint64> uint128;

inline uint64 Uint128Low64(const uint128& x) { return x.first; }
inline uint64 Uint128High64(const uint128& x) { return x.second; }

// Hash function for a byte array.
uint64 CityHash64(const char *buf, size_t len);

// Hash function for a byte array.  For convenience, a 64-bit seed is also
// hashed into the result.
uint64 CityHash64WithSeed(const char *buf, size_t len, uint64 seed);

// Hash function for a byte array.  For convenience, two seeds are also
// hashed into the result.
uint64 CityHash64WithSeeds(const char *buf, size_t len,
                           uint64 seed0, uint64 seed1);

// Hash function for a byte array.
uint128 CityHash128(const char *s, size_t len);

// Hash function for a byte array.  For convenience, a 128-bit seed is also
// hashed into the result.
uint128 CityHash128WithSeed(const char *s, size_t len, uint128 seed);

// Hash function for a byte array.  Most useful in 32-bit binaries.
uint32 CityHash32(const char *buf, size_t len);

// Hash 128 input bits down to 64 bits of output.
// This is intended to be a reasonably good hash function.
inline uint64 Hash128to64(const uint128& x) {
    // Murmur-inspired hashing.
    const uint64 kMul = 0x9ddfea08eb382d69ULL;
    uint64 a = (Uint128Low64(x) ^ Uint128High64(x)) * kMul;
    a ^= (a >> 47);
    uint64 b = (Uint128High64(x) ^ a) * kMul;
    b ^= (b >> 47);
    b *= kMul;
    return b;
}


typedef uint64 HashType; // aka size_t

inline HashType hash(const char *buf, size_t len) {
    return CityHash64(buf, len);
}

inline HashType hash(const uint32_t *buf, size_t len) {
    return CityHash64(reinterpret_cast<const char*>(buf), len);
}

inline HashType hashWithSeed(const char *buf, size_t len, uint64 seed) {
    return CityHash64WithSeed(buf, len, seed);
}

inline HashType hashWithSeed(const uint32_t *buf, size_t len, uint64 seed) {
    return CityHash64WithSeed(reinterpret_cast<const char*>(buf), len, seed);
}



/**
 * Calculate hash size to preallocate buffer, in which each element is copied.
 * Pass this buffer to CityHash. Export only relevant hashCombine to not pollute namespace.
 */
namespace _hashing {

using std::array;
using std::declval;
using std::enable_if;
using std::is_arithmetic;
using std::is_same;
using std::pair;
using std::shared_ptr;
using std::vector;

// Signatures for size
inline size_t _hashCombineSize() { return 0; }
template<typename... Rest>
inline size_t _hashCombineSize(const std::string &v, Rest... rest);
template<typename T, typename... Rest>
inline typename enable_if<is_arithmetic<T>::value, size_t>::type
_hashCombineSize(T v, Rest... rest);
template<typename Hashable, typename... Rest>
inline typename enable_if<is_same<HashType,decltype(declval<shared_ptr<Hashable>>()->getHash())>::value, size_t>::type
_hashCombineSize(const shared_ptr<Hashable> &v, Rest... rest);
template<typename Hashable, typename... Rest>
inline typename enable_if<!is_same<HashType,decltype(declval<shared_ptr<Hashable>>()->getHash())>::value, size_t>::type
_hashCombineSize(const shared_ptr<Hashable> &v, Rest... rest);
template<typename Hashable, typename... Rest>
inline typename enable_if<is_same<HashType,decltype(declval<Hashable>().getHash())>::value,size_t>::type
_hashCombineSize(const Hashable &v, Rest... rest);
template<typename Hashable, typename... Rest>
inline size_t _hashCombineSize(const vector<Hashable> &v, Rest... rest);
template<typename Hashable, size_t Num, typename... Rest>
inline size_t _hashCombineSize(const array<Hashable, Num> &v, Rest... rest);
template<typename Hashable, typename... Rest>
inline size_t _hashCombineSize(const pair<Hashable, Hashable> &v, Rest... rest);

// Implementations for size
template<typename... Rest>
inline size_t _hashCombineSize(const std::string &v, Rest... rest) {
    return v.size() + _hashCombineSize(rest...);
}
template<typename T, typename... Rest>
inline typename enable_if<is_arithmetic<T>::value, size_t>::type
_hashCombineSize(T v, Rest... rest) {
    return sizeof(v) + _hashCombineSize(rest...);
}
template<typename Hashable, typename... Rest>
inline typename enable_if<is_same<HashType,decltype(declval<shared_ptr<Hashable>>()->getHash())>::value,size_t>::type
_hashCombineSize(const shared_ptr<Hashable> &v, Rest... rest) {
    UNUSED_EXPR(v);
    return sizeof(HashType) + _hashCombineSize(rest...);
}
template<typename Hashable, typename... Rest>
inline typename enable_if<!is_same<HashType,decltype(declval<shared_ptr<Hashable>>()->getHash())>::value,size_t>::type
_hashCombineSize(const shared_ptr<Hashable> &v, Rest... rest) {
    return _hashCombineSize(*v) + _hashCombineSize(rest...);
}
template<typename Hashable, typename... Rest>
inline typename enable_if<is_same<HashType,decltype(declval<Hashable>().getHash())>::value,size_t>::type
_hashCombineSize(const Hashable &v, Rest... rest) {
    UNUSED_EXPR(v);
    return sizeof(HashType) + _hashCombineSize(rest...);
}
template<typename Hashable, typename... Rest>
inline size_t _hashCombineSize(const vector<Hashable> &v, Rest... rest) {
    size_t len = 0;
    for (const auto &item : v) {
        len += _hashCombineSize(item);
    }
    return len + _hashCombineSize(rest...);
}
template<typename Hashable, size_t Num, typename... Rest>
inline size_t _hashCombineSize(const array<Hashable, Num> &v, Rest... rest) {
    size_t len = 0;
    for (const auto &item : v) {
        len += _hashCombineSize(item);
    }
    return len + _hashCombineSize(rest...);
}
template<typename Hashable, typename... Rest>
inline size_t _hashCombineSize(const pair<Hashable, Hashable> &v, Rest... rest) {
    return _hashCombineSize(v.first) + _hashCombineSize(v.second) + _hashCombineSize(rest...);
}

// Signatures for copy
inline void _hashCpy(char *buf, size_t offset)
{ UNUSED_EXPR(buf); UNUSED_EXPR(offset); } // unused because it is base case of copy
template<typename... Rest>
inline void _hashCpy(char *buf, size_t offset, const std::string &v, Rest... rest);
template<typename T, typename... Rest>
inline typename enable_if<is_arithmetic<T>::value, void>::type
_hashCpy(char *buf, size_t offset, T v, Rest... rest);
template<typename Hashable, typename... Rest>
inline typename enable_if<is_same<HashType,decltype(declval<shared_ptr<Hashable>>()->getHash())>::value,void>::type
_hashCpy(char *buf, size_t offset, const shared_ptr<Hashable> &v, Rest... rest);
template<typename Hashable, typename... Rest>
inline typename enable_if<!is_same<HashType,decltype(declval<shared_ptr<Hashable>>()->getHash())>::value,void>::type
_hashCpy(char *buf, size_t offset, const shared_ptr<Hashable> &v, Rest... rest);
template<typename Hashable, typename... Rest>
inline typename enable_if<is_same<HashType,decltype(declval<Hashable>().getHash())>::value,void>::type
_hashCpy(char *buf, size_t offset, const Hashable &v, Rest... rest);
template<typename Hashable, typename... Rest>
inline void _hashCpy(char *buf, size_t offset, const vector<Hashable> &v, Rest... rest);
template<typename Hashable, size_t Num, typename... Rest>
inline void _hashCpy(char *buf, size_t offset, const array<Hashable, Num> &v, Rest... rest);
template<typename Hashable, typename... Rest>
inline void _hashCpy(char *buf, size_t offset, const pair<Hashable, Hashable> &v, Rest... rest);

// Implementations for copy
template<typename... Rest>
inline void _hashCpy(char *buf, size_t offset, const std::string &v, Rest... rest) {
    memcpy(buf + offset, v.data(), v.size());
    _hashCpy(buf, offset + v.size(), rest...);
}
template<typename T, typename... Rest>
inline typename enable_if<is_arithmetic<T>::value, void>::type
_hashCpy(char *buf, size_t offset, T v, Rest... rest) {
    memcpy(buf + offset, &v, sizeof(v));
    _hashCpy(buf, offset + sizeof(v), rest...);
}
template<typename Hashable, typename... Rest>
inline typename enable_if<is_same<HashType, decltype(declval<shared_ptr<Hashable>>()->getHash())>::value,void>::type
_hashCpy(char *buf, size_t offset, const shared_ptr<Hashable> &v, Rest... rest) {
    HashType hash = v != nullptr ? v->getHash() : 0;
    memcpy(buf + offset, &hash, sizeof(hash));
    _hashCpy(buf, offset + sizeof(hash), rest...);
}
template<typename Hashable, typename... Rest>
inline typename enable_if<!is_same<HashType,decltype(declval<shared_ptr<Hashable>>()->getHash())>::value,void>::type
_hashCpy(char *buf, size_t offset, const shared_ptr<Hashable> &v, Rest... rest) {
    _hashCpy(buf, offset, *v);
    UNUSED_VARIADIC(rest);
}
template<typename Hashable, typename... Rest>
inline typename enable_if<is_same<HashType, decltype(declval<Hashable>().getHash())>::value,void>::type
_hashCpy(char *buf, size_t offset, const Hashable &v, Rest... rest) {
    HashType hash = v.getHash();
    memcpy(buf + offset, &hash, sizeof(hash));
    _hashCpy(buf, offset + sizeof(hash), rest...);
}
template<typename Hashable, typename... Rest>
inline void _hashCpy(char *buf, size_t offset, const vector<Hashable> &v, Rest... rest) {
    for (const auto &item : v) {
        _hashCpy(buf, offset, item);
        offset += _hashCombineSize(item);
    }
    _hashCpy(buf, offset, rest...);
}
template<typename Hashable, size_t Num, typename... Rest>
inline void _hashCpy(char *buf, size_t offset, const array<Hashable, Num> &v, Rest... rest) {
    for (const auto &item : v) {
        _hashCpy(buf, offset, item);
        offset += _hashCombineSize(item);
    }
    _hashCpy(buf, offset, rest...);
}
template<typename Hashable, typename... Rest>
inline void _hashCpy(char *buf, size_t offset, const pair<Hashable, Hashable> &v, Rest... rest) {
    _hashCpy(buf, offset, v.first);
    offset += _hashCombineSize(v.first);
    _hashCpy(buf, offset, v.second);
    offset += _hashCombineSize(v.second);
    _hashCpy(buf, offset, rest...);
}

// final hashCombine
template<typename... Rest>
inline HashType hashCombine(HashType seed, Rest... rest) {
    size_t bufSize = _hashCombineSize(rest...);
    char *buf = new char[bufSize];
    _hashCpy(buf, 0, rest...);
    HashType h = hashWithSeed(buf, bufSize, seed);
    delete[] buf;
    return h;
}

}

using _hashing::hashCombine;

}

#endif  // CITY_HASH_H_
