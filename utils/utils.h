//
// Created by rozliv on 15.08.2017.
//

#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#include <iostream>
#include <vector>
#include <memory>


using std::vector;
using std::move;
using std::unique_ptr;
using std::shared_ptr;


// CartProduct returns cartesian product of all items in a vector
template<typename T>
vector<vector<T>> CartProduct(const vector<vector<T>> &v) {
  vector<vector<T>> s = {{}};
  for (const auto& u : v) {
    vector<vector<T>> r;
    for (const auto& x : s) {
      for (const auto& y : u) {
        r.push_back(x);
        r.back().push_back(y);
      }
    }
    s = move(r);
  }
  return s;
}

/* CastDynamic dynamically casts vector of type T to vector of type U,
 * works only with shared_ptr, it has no problem with virtual inheritence.
 */
template <typename T, typename U>
vector<shared_ptr<U>> CastDynamic(const vector<shared_ptr<T>>& list2) {
  vector<shared_ptr<U>> list = vector<shared_ptr<U>>();
  for (const auto &j : list2) {
    list.push_back(std::dynamic_pointer_cast<U>(j));
  }
  return list;
}


/* Cast statically casts vector of type T to vector of type U,
 * works only with shared_ptr, it should be a default choice.
 */
template <typename T, typename U>
vector<shared_ptr<U>> Cast(const vector<shared_ptr<T>>& list2) {
  vector<shared_ptr<U>> list = vector<shared_ptr<U>>();
  for (const auto &j : list2) {
    list.push_back(std::static_pointer_cast<U>(j));
  }
  return list;
}

// overloaded operator + for vector<double>
inline vector<double> operator+(const vector<double>& v1, const vector<double>& v2) {
  if (v1.size() == v2.size()) {
    vector<double> vec = vector<double>(v1.size());
    for (unsigned int k = 0; k < v1.size(); ++k) {
      vec[k] = v1[k] + v2[k];
    }
    return vec;
  }
  return {};
}

// overloaded operator += for vector<double>
inline vector<double>& operator +=(vector<double>& v1, const vector<double>& v2) {
  if (v1.size() == v2.size()) {
    for (unsigned int k = 0; k < v1.size(); ++k) {
      v1[k] += v2[k];
    }
  }
  return v1;
}


/*
 * Hash container for using vector in a hash map
 */

namespace std {
template<>  // we can make this generic for any container [1]
struct hash<vector<int>> {
  std::size_t operator()(vector<int> const &c) const {
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
