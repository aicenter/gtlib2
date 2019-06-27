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

#include <iostream>
#include <vector>
#include <memory>
#include <cmath>


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

}  // namespace std

namespace GTLib2 {
inline bool is_negative_zero(float val) { return ((val == 0.0f) && std::signbit(val)); }
inline bool is_negative_zero(double val) { return ((val == 0.0) && std::signbit(val)); }
inline bool is_positive_zero(float val) { return ((val == 0.0f) && std::signbit(val)); }
inline bool is_positive_zero(double val) { return ((val == 0.0) && std::signbit(val)); }
}

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


#endif  // UTILS_UTILS_H_
