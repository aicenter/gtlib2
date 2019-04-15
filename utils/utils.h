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

}  // namespace std
#endif  // UTILS_UTILS_H_
