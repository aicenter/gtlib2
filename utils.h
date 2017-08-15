//
// Created by rozliv on 15.08.2017.
//

#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <vector>
#include <memory>


using std::vector;
using std::move;
using std::unique_ptr;
using std::shared_ptr;

template<typename T>
vector<vector<shared_ptr<T>>> cart_product(const vector<vector<shared_ptr<T>>>& v) {
  vector<vector<shared_ptr<T>>> s = {{}};
  for (const auto& u : v) {
    vector<vector<shared_ptr<T>>> r;
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

template<typename T, typename ...Args>
unique_ptr<T> make_uniq(Args&&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename U>
vector<shared_ptr<U>> Cast(const vector<shared_ptr<T>>& list2) {
  vector<shared_ptr<U>> list = vector<shared_ptr<U>>();
  for (const auto &j : list2) {
    list.push_back(std::dynamic_pointer_cast<U>(j));
  }
  return list;
}


#endif  // UTILS_H_
