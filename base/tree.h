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


#ifndef GTLIB2_TREE_H
#define GTLIB2_TREE_H

#include "base/includes.h"
#include "base/hashing.h"
#include "utils/utils.h"

namespace GTLib2 {

typedef uint32_t EdgeId;

template<class Parent, class Child>
class Node {
// This can't be used unfortunately: template is instantiated by a class that's inheriting it.
// This calss is only forward declared and can be used only to hold the pointer or reference.
//    static_assert(std::is_base_of<Node, Parent>::value, "Parent must derive from the Node class");
//    static_assert(std::is_base_of<Node, Child>::value, "Child must derive from the Node class");
 public:
    explicit Node(shared_ptr<Parent const> parent, EdgeId incomingEdge) :
        parent_(move(parent)),
        depth_(parent_ == nullptr ? 0 : parent_->depth_ + 1),
        incomingEdge_(incomingEdge),
        history_(parent_ == nullptr
                 ? vector<EdgeId>()
                 : extend(parent_->history_, incomingEdge)),
        hashNode_(hashWithSeed(history_.data(), history_.size() * sizeof(EdgeId), 1412914847)) {}

    virtual unsigned int countChildren() = 0;
    virtual const shared_ptr <Child> getChildAt(EdgeId index) = 0;

    // used for template overloading in hashCombine
    inline HashType getHash() const { return hashNode_; };

    inline bool operator==(const Node &rhs) const {
        if (hashNode_ != rhs.hashNode_) return false;
        if (history_.size() != rhs.history_.size()) return false;
        return !memcmp(history_.data(), rhs.history_.data(), history_.size());
    }

    /**
     * Return textual representation of node history
     */
    string toString() const {
        std::stringstream ss;
        ss << "∅";
        for (unsigned int edgeId : history_) ss << "," << edgeId;
        return ss.str();
    }

    // todo: friend ostream (doesn't work, dunno why)
//    friend std::ostream &
//    operator<<(std::ostream &ss, const Node<Parent, Child> &node) {
//        ss << "∅";
//        for (unsigned int edgeId : node.history_) ss << "," << edgeId;
//        return ss;
//    }

    const shared_ptr<Parent const> parent_;
    const unsigned int depth_;
    const EdgeId incomingEdge_;
    const vector <EdgeId> history_;
 private:
    const HashType hashNode_;
};

template<class N>
using NodeCallback = function<void(shared_ptr < N > )>;

/**
 * Call supplied function at each node of the tree, including leaves.
 * The tree is walked as DFS up to maximum specified depth.
 */
template<class N>
void treeWalk(const shared_ptr <N> &node, const NodeCallback<N> &function, int maxDepth) {
    function(node);
    if (node->depth_ >= maxDepth) return;
    for (EdgeId i = 0; i < node->countChildren(); ++i) {
        treeWalk(node->getChildAt(i), function, maxDepth);
    }
}

/**
 * Call supplied function at each node of the tree, including leaves,
 * with no restriction to depth of the tree walk.
 */
template<class N>
void treeWalk(const shared_ptr <N> &node, const NodeCallback<N> &function) {
    function(node);
    for (EdgeId i = 0; i < node->countChildren(); ++i) {
        treeWalk(node->getChildAt(i), function);
    }
}

}

#endif //GTLIB2_TREE_H
