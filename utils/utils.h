/*
    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague

    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#include <cassert>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <string.h>

/**
 * CartProduct returns cartesian product of all items in a vector
 */
template<typename T>
std::vector<std::vector<T>> CartProduct(const std::vector<std::vector<T>> &v) {
    std::vector<std::vector<T>> s = {{}};
    for (const auto &u : v) {
        std::vector<std::vector<T>> r;
        for (const auto &x : s) {
            for (const auto &y : u) {
                r.push_back(x);
                r.back().push_back(y);
            }
        }
        s = std::move(r);
    }
    return s;
}

/**
 * CastDynamic dynamically casts vector of type T to vector of type U,
 * works only with shared_ptr, it has no problem with virtual inheritence.
 */
template<typename T, typename U>
std::vector<std::shared_ptr<U>> CastDynamic(const std::vector<std::shared_ptr<T>> &list2) {
    std::vector<std::shared_ptr<U>> list = std::vector<std::shared_ptr<U>>();
    for (const auto &j : list2) {
        list.push_back(std::dynamic_pointer_cast<U>(j));
    }
    return list;
}


/**
 * Cast statically casts vector of type T to vector of type U,
 * works only with std::shared_ptr, it should be a default choice.
 */
template<typename T, typename U>
std::vector<std::shared_ptr<U>> Cast(const std::vector<std::shared_ptr<T>> &list2) {
    std::vector<std::shared_ptr<U>> list = std::vector<std::shared_ptr<U>>();
    for (const auto &j : list2) {
        list.push_back(std::static_pointer_cast<U>(j));
    }
    return list;
}

/**
 * Overloaded operator + for vector<double>
 */
inline std::vector<double> operator+(const std::vector<double> &v1, const std::vector<double> &v2) {
    if (v1.size() == v2.size()) {
        std::vector<double> vec = std::vector<double>(v1.size());
        for (unsigned int k = 0; k < v1.size(); ++k) {
            vec[k] = v1[k] + v2[k];
        }
        return vec;
    }
    return {};
}

/**
 * overloaded operator += for vector<double>
 */
inline std::vector<double> &operator+=(std::vector<double> &v1, const std::vector<double> &v2) {
    if (v1.size() == v2.size()) {
        for (unsigned int k = 0; k < v1.size(); ++k) {
            v1[k] += v2[k];
        }
    }
    return v1;
}

/**
 * Elegant pairing and unpairing functions by Matthew Szudzik @ Wolfram Research, Inc.
 * Gives unique integers for pair of positive integers, range <(0, 0), (65535, 65535)>
 */
inline unsigned int elegantPair(unsigned int a, unsigned int b) {
    assert(a <= 65535);
    assert(b <= 65535);
    return a >= b ? a * a + a + b : a + b * b;
}

inline std::array<unsigned int, 2> elegantUnpair(unsigned int z) {
    unsigned sqrtZ = floor(sqrt(z));
    unsigned sqSqrtZ = sqrtZ * sqrtZ;
    if ((z - sqSqrtZ) >= sqrtZ) {
        return {sqrtZ, z - sqSqrtZ - sqrtZ};
    }
    return {z - sqSqrtZ, sqrtZ};
}

/**
 * Returns signum of given number, e.g. -1 if number < 0, 0 if number == 0, 1 if number > 0.
 * @param number
 * @return
 */
inline double signum(double number) {
    return (double) ((0 < number) - (0 > number));
}

/**
 * Returns given boolean as string.
 * @param expr
 * @return
 */
inline std::string boolToString(bool expr) {
    return expr ? "True" : "False";
}


namespace std {

/**
 * Hash container for using vector in a hash map
 */
template<>  // we can make this generic for any container [1]
struct hash<std::vector<int>> {
    std::size_t operator()(std::vector<int> const &c) const {
        std::size_t result = c.size();

        std::hash<int> h;

        for (int x : c) {
            result += result * 31 + h(x);
        }
        return result;
    }
};

template<typename T, typename... Ts>
std::vector<T> extend(const std::vector<T>& original, const Ts&... args) {
    constexpr std::size_t numArgs = sizeof...(Ts);
    std::vector<T> made;
    made.reserve(original.size() + numArgs);
    for (unsigned int i = 0; i < original.size(); i++) made.push_back(original[i]);
    (made.push_back(args), ...);
    return made;
}

template<typename T>
bool isExtension(const vector<T> &base, const vector<T> &extending) {
    auto sizeTarget = base.size();
    auto sizeCmp = extending.size();
    if(sizeCmp <= sizeTarget) return false;

    size_t cmpBytes = min(sizeTarget, sizeCmp) * sizeof(T);
    return !memcmp(base.data(), extending.data(), cmpBytes);
}

}  // namespace std

namespace GTLib2 {
inline bool is_negative_zero(float val) { return ((val == 0.0f) && std::signbit(val)); }
inline bool is_negative_zero(double val) { return ((val == 0.0) && std::signbit(val)); }
inline bool is_positive_zero(float val) { return ((val == 0.0f) && std::signbit(val)); }
inline bool is_positive_zero(double val) { return ((val == 0.0) && std::signbit(val)); }

struct Escaped {
    std::string str;

    friend inline std::ostream& operator<<(std::ostream& os, const Escaped& e) {
        for(const char &c : e.str) {
            switch (c) {
                case '\a':  os << "\\a"; break;
                case '\b':  os << "\\b"; break;
                case '\f':  os << "\\f"; break;
                case '\n':  os << "\\n"; break;
                case '\r':  os << "\\r"; break;
                case '\t':  os << "\\t"; break;
                case '\v':  os << "\\v"; break;
                case '\\':  os << "\\\\"; break;
                case '\'':  os << "\\'"; break;
                case '\"':  os << "\\\""; break;
                case '\?':  os << "\\\?"; break;
                default: os << c;
            }
        }
        return os;
    }
};

template<typename A, typename B>
struct Either {
    bool cond;
    const A& a;
    const B& b;
    Either(bool _cond, const A& _a, const B&_b) : cond(_cond), a(_a), b(_b) {}

    friend inline std::ostream& operator<<(std::ostream& os, const Either& e) {
        if(e.cond) os << e.a;
        else os << e.b;
        return os;
    }
};

struct Indented {
    std::string str;
    int times;

    friend inline std::ostream& operator<<(std::ostream& os, const Indented& e) {
        for (int i = 0; i < e.times; ++i) os << ' ';
        for(const char &c : e.str) {
            if(c == '\n') {
                os << '\n';
                for (int i = 0; i < e.times; ++i) os << ' ';
            } else {
                os << c;
            }
        }
        return os;
    }
};

}

#define UNUSED_EXPR(expr) do { (void)(expr); } while (0)
#define UNUSED_VARIADIC(expr) do { (void)(sizeof...(expr)); } while (0)

#define MAKE_HASHABLE(type)                                           \
    namespace std {                                                   \
        template<> struct hash<type> {                                \
            size_t operator()(const type &t) const {                  \
                return t.getHash();                                   \
            }                                                         \
        };                                                            \
        template<> struct hash<type*> {                               \
            size_t operator()(type* &t) const {                       \
                return t->getHash();                                  \
            }                                                         \
        };                                                            \
        template<> struct hash<shared_ptr<type>> {                    \
            size_t operator()(const shared_ptr<type> &t) const {      \
                return t->getHash();                                  \
            }                                                         \
        };                                                            \
        template<> struct hash<unique_ptr<type>> {                    \
            size_t operator()(const unique_ptr<type> &t) const {      \
                return t->getHash();                                  \
            }                                                         \
        };                                                            \
    }

#define MAKE_EQ(type)                                                 \
    namespace std {                                                   \
        template<> struct equal_to<type> {                            \
            bool operator()(const type &a, const type &b) const {     \
                return a == b;                                        \
            }                                                         \
        };                                                            \
        template<> struct equal_to<type*> {                           \
            bool operator()(const type* &a, const type* &b) const {   \
                return *a == *b;                                      \
            }                                                         \
        };                                                            \
        template<> struct equal_to<shared_ptr<type>> {                \
            bool operator()(const shared_ptr<type> &a,                \
                            const shared_ptr<type> &b) const {        \
                return *a == *b;                                      \
            }                                                         \
        };                                                            \
        template<> struct equal_to<unique_ptr<type>> {                \
            bool operator()(const unique_ptr<type> &a,                \
                            const unique_ptr<type> &b) const {        \
                return *a == *b;                                      \
            }                                                         \
        };                                                            \
    }


#define _TYPE_DERIVE_OF_1(T, A)       std::is_base_of<A, T>::value
#define _TYPE_DERIVE_OF_2(T, A, B)    std::is_base_of<A, T>::value && std::is_base_of<B, T>::value
#define _TYPE_DERIVE_OF_3(T, A, ...)  std::is_base_of<A, T>::value && _TYPE_DERIVE_OF_2(T, __VA_ARGS__)
#define _TYPE_DERIVE_OF_4(T, A, ...)  std::is_base_of<A, T>::value && _TYPE_DERIVE_OF_3(T, __VA_ARGS__)
#define _TYPE_DERIVE_OF_5(T, A, ...)  std::is_base_of<A, T>::value && _TYPE_DERIVE_OF_4(T, __VA_ARGS__)
#define _TYPE_DERIVE_OF_6(T, A, ...)  std::is_base_of<A, T>::value && _TYPE_DERIVE_OF_5(T, __VA_ARGS__)
#define _TYPE_DERIVE_OF_7(T, A, ...)  std::is_base_of<A, T>::value && _TYPE_DERIVE_OF_6(T, __VA_ARGS__)
#define _TYPE_DERIVE_OF_8(T, A, ...)  std::is_base_of<A, T>::value && _TYPE_DERIVE_OF_7(T, __VA_ARGS__)
#define _TYPE_DERIVE_OF_9(T, A, ...)  std::is_base_of<A, T>::value && _TYPE_DERIVE_OF_8(T, __VA_ARGS__)

// NUM_ARGS(...) evaluates to the literal number of the passed-in arguments.
#define _NUM_ARGS2(X, X10, X9, X8, X7, X6, X5, X4, X3, X2, X1, N, ...) N
#define NUM_ARGS(...) _NUM_ARGS2(0, __VA_ARGS__ ,10,9,8,7,6,5,4,3,2,1,0)

#define _TYPE_DERIVE_OF_N3(T, N, ...)  _TYPE_DERIVE_OF_ ## N(T, __VA_ARGS__)
#define _TYPE_DERIVE_OF_N2(T, N, ...)  _TYPE_DERIVE_OF_N3(T, N, __VA_ARGS__)

// Syntactic sugar for templates (multiple inheritance types)
#define TYPE_DERIVES_FROM(T, ...)                                  \
    typename = typename std::enable_if<                            \
        _TYPE_DERIVE_OF_N2(T, NUM_ARGS(__VA_ARGS__), __VA_ARGS__)  \
    >::type


#ifndef NDEBUG
#define unreachable(MSG) \
  (assert(0 && MSG), abort())
#else
#define unreachable(MSG) \
  (cerr << "UNREACHABLE executed at " << __FILE__<<":"<<__LINE__<< " with error " << MSG << endl, abort())
#endif

namespace std { // NOLINT(cert-dcl58-cpp)

template<typename T>
std::ostream &operator<<(std::ostream &ss, const vector<T> &arr) {
    ss << "[";
    for (unsigned int i = 0; i < arr.size(); ++i) {
        if (i == 0) ss << arr.at(i);
        else ss << ", " << arr.at(i);
    }
    ss << "]";
    return ss;
}

template<typename K, typename V>
std::ostream &operator<<(std::ostream &ss, const std::unordered_map<K, V> &map) {
    bool addNewLine = map.size() > 4;
    ss << "{";
    if(addNewLine) ss << endl;
    bool first=true;
    for (const auto &[k,v] : map) {
        if(!first && !addNewLine) ss << ", ";
        ss << k << ": " << v;
        first=false;
        if(addNewLine) ss << endl;
    }
    ss << "}";
    return ss;
}

template<typename K, typename V>
std::ostream &operator<<(std::ostream &ss, const std::unordered_map<shared_ptr<K>, V> &map) {
    bool addNewLine = map.size() > 4;
    ss << "{";
    if(addNewLine) ss << endl;
    bool first=true;
    for (const auto &[k,v] : map) {
        if(!first && !addNewLine) ss << ", ";
        ss << *k << ": " << v;
        first=false;
        if(addNewLine) ss << endl;
    }
    ss << "}";
    return ss;
}

}  // namespace std

#endif  // UTILS_UTILS_H_
