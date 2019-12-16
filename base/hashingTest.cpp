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

    You should have received a copy of the GNU Lesser General Public 
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/



#include "base/base.h"
#include "base/hashing.h"
#include "gtest/gtest.h"
#include "domains/pursuit.h"


namespace GTLib2 {
class A {
 public:
    explicit A(int x) : x_(x) {};
    int x_;
    const HashType getHash() const { return x_; }
};

TEST(Hashing, CallHashCombine) {
    HashType val = 7725800078484773444;

    EXPECT_EQ(hashCombine(10, A(1), A(2), A(3)), val);
    EXPECT_EQ(hashCombine(10, A(1), A(2), A(3)), val);
    EXPECT_EQ(hashCombine(10, A(1), vector<A>{A(2), A(3)}), val);
    EXPECT_EQ(hashCombine(10, make_shared<A>(1), vector<A>{A(2), A(3)}), val);
    EXPECT_EQ(hashCombine(10, A(1), vector<shared_ptr<A>>{
        make_shared<A>(2), make_shared<A>(3)}), val);
    EXPECT_EQ(hashCombine(10, A(1), vector<vector<shared_ptr<A>>>{
        {make_shared<A>(2)}, {make_shared<A>(3)}}), val);
    EXPECT_EQ(hashCombine(10, vector<shared_ptr<A>>
        {make_shared<A>(1), make_shared<A>(2), make_shared<A>(3)}), val);
    EXPECT_EQ(hashCombine(10, vector<vector<shared_ptr<A>>>{
        {make_shared<A>(1)}, {make_shared<A>(2), make_shared<A>(3)}}), val);

    EXPECT_EQ(hashCombine(10, vector<pair<shared_ptr<A>, shared_ptr<A>>>
        {make_pair(make_shared<A>(1), make_shared<A>(2))}, A(3)), val);


    EXPECT_EQ(hashCombine(10, vector<vector<shared_ptr<A>>>{
        {make_shared<A>(1)},
        {make_shared<A>(2), make_shared<A>(3)},
    }), val);

    EXPECT_EQ(hashCombine(10, vector<pair<shared_ptr<A>, shared_ptr<A>>>
        {make_pair(make_shared<A>(1), make_shared<A>(2))}, make_shared<A>(3)), val);

    EXPECT_EQ(hashCombine(10, uint64(1), uint64(2), uint64(3)), val);

    using namespace std::string_literals;
    string x = "\x01\x00\x00\x00\x00\x00\x00\x00"s
               "\x02\x00\x00\x00\x00\x00\x00\x00"s
               "\x03\x00\x00\x00\x00\x00\x00\x00"s;
    EXPECT_EQ(hashCombine(10, x), val);
    std::cout << std::setw(16) << std::setfill('0') << std::hex << uint64(1);
}

}
