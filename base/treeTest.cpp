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


#ifndef GTLIB2_TREETEST_CPP
#define GTLIB2_TREETEST_CPP

#include "base/tree.h"
#include "gtest/gtest.h"

namespace GTLib2 {

/**
 * Simple example binary tree of depth 2
 */
class ExampleNode: public Node<ExampleNode>,
                   public std::enable_shared_from_this<ExampleNode const> {
 public:
    ExampleNode(shared_ptr<ExampleNode const> parent, optional<EdgeId> incomingEdge)
        : Node(move(parent), incomingEdge) {}

};

template<>
unsigned int nodeChildCnt<ExampleNode>(const shared_ptr<ExampleNode> &node) {
    if (node->getDepth() < 2) return 2;
    return 0;
}

template<>
inline shared_ptr<ExampleNode> nodeChildExpander<ExampleNode>(const shared_ptr<ExampleNode> &node, EdgeId index) {
    return make_shared<ExampleNode>(node->shared_from_this(), index);
}

TEST(Tree, WalkBinaryTree) {
    std::stringstream ss;
    auto callback = [&](shared_ptr<ExampleNode> node) { ss << node->toString() << "\n"; };
    auto rootNode = make_shared<ExampleNode>(nullptr, nullopt);
    treeWalk<ExampleNode>(rootNode, callback);

    string expectedTree = "∅\n"
                          "∅,0\n"
                          "∅,0,0\n"
                          "∅,0,1\n"
                          "∅,1\n"
                          "∅,1,0\n"
                          "∅,1,1\n";

    EXPECT_EQ(ss.str(), expectedTree);
}

}


#endif //GTLIB2_TREETEST_CPP
