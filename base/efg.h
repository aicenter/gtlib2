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


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef BASE_EFG_H_
#define BASE_EFG_H_

#include <experimental/optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <string>
#include <functional>

#include "base/base.h"

using std::unordered_map;
using std::unordered_set;
using std::experimental::nullopt;
using std::experimental::optional;


namespace GTLib2 {

class EFGNode;
class EFGCache;

/**
 * The chance probability of an EFGNode after performing some action,
 * i.e. not from the root, but from a parent node `h` to it's child `g` -- $\pi_c(g|h)$.
 */
typedef pair<shared_ptr<EFGNode>, double> EFGDistEntry;

/**
 * Entire probability distribution over the next EFGNodes after performing some action.
 *
 * This should sum up to 1.
 */
typedef vector<EFGDistEntry> EFGNodesDistribution;

/**
 * Distribution of nodes after following a specified action id.
 *
 * Note that action ids are indexed from 0..N-1
 */
typedef vector<shared_ptr<EFGNodesDistribution>> EFGActionNodesDistribution;

/**
 * EFGNode is a class that represents node in an extensive form game (EFG).
 *
 * It contains:
 * - action-observation history,
 * - state,
 * - rewards (utility) and
 * - information set.
 *
 * There are two types of EFGNodes: a) inner nodes and b) terminal nodes.
 * They can be distinguished by calling isTerminal() method.
 *
 * Chance nodes are encoded into the distribution of next nodes (i.e. EFGNodesDistribution)
 * after performing specified action.
 *
 * Note that **many calls are not cached!** Use EFGCache to save the tree structure,
 * and iteratore over the tree using the EFGCache::getChildrenFor() method.
 */
class EFGNode final: public std::enable_shared_from_this<EFGNode const> {
 public:
    /**
     * Constructor for the same round node
     */
    EFGNode(shared_ptr<EFGNode const> parent,
            const vector<PlayerAction> &performedActions,
            shared_ptr<Action> incomingAction, int depth);

    /**
     * Constructor for the new round node
     */
    EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
            const vector<shared_ptr<Observation>> &observations,
            const vector<double> &rewards,
            double natureProbability, shared_ptr<Action> incomingAction, int depth);

    /**
     * Returns the sequence of actions performed by the player since the root.
     */
    ActionSequence getActionsSeqOfPlayer(int player) const;

    double getProbabilityOfActionsSeqOfPlayer(int player, const BehavioralStrategy &strat) const;

    /**
     * Returns available actions for the current player
     * Leaves do not have any available actions.
     */
    vector<shared_ptr<Action>> availableActions() const;

    /**
     * Perform the given action and returns the next node
     * or nodes in case of stochastic games together with the probabilities.
     */
    EFGNodesDistribution performAction(const shared_ptr<Action> &action) const;

    /**
     * Gets the information set of the node represented as ActionObservationHistory set.
     */
    shared_ptr<AOH> getAOHInfSet() const;

    /**
     * Gets the augmented information set of the node.
     *
     * Note that augmented information sets coincide with ordinary information sets
     * when the requested player is acting in this node.
     */
    shared_ptr<AOH> getAOHAugInfSet(Player player) const;

    /**
     * Check if the node is in the given information set.
     */
    bool isContainedInInformationSet(const shared_ptr<AOH> &infSet) const;

    /**
     * Gets the parent efg node.
     */
    shared_ptr<EFGNode const> getParent() const;

    /**
     * Gets action that was performed at parent node and the result led to this node.
     */
    const shared_ptr<Action> &getIncomingAction() const;

    /**
     * Gets id of action that was performed at parent node and the result led to this node.
     */
    ActionId getIncomingActionId() const;

    /**
     * Returns the game state of that is represented by EFG node.
     * Note that in simultaneous games one state corresponds to multiple efg nodes.
     */
    shared_ptr<State> getState() const;

    /**
     * Check if NO actions were played in this round
     */
    bool noActionPerformedInThisRound() const;

    /**
     * Return if this node is terminal (leaf)
     */
    bool isTerminal() const;

    string toString() const;

    size_t getHash() const;

    /**
     * Get the depth in the sense of State depth
     */
    int getDepth() const;

    /**
     * Get the depth in the sense of EFG depth
     */
    int getDistanceFromRoot() const;

    ObservationId getLastObservationIdOfCurrentPlayer() const;

    /**
     * Number of remaining players in corresponding State
     */
    int getNumberOfRemainingPlayers() const;

    ObservationId getLastObservationOfPlayer(Player player) const;

    /**
     * Get current player playing in this round.
     * Leaves do not have a player and no value is present.
     */
    optional<Player> getCurrentPlayer() const;

    vector<double> rewards_;

    /**
     * Nature reach probability from root
     */
    double natureProbability_;

    bool operator==(const EFGNode &rhs) const;

 private:
    vector<ActionObservation> getAOH(Player player) const;
    bool compareAOH(const EFGNode &rhs) const;
    size_t getHashedAOHs() const;
    vector<shared_ptr<Observation>> observations_;
    vector<PlayerAction> performedActionsInThisRound_;
    vector<Player> remainingPlayersInTheRound_;
    // todo: const for member variables and constructors?
    shared_ptr<State> state_;
    shared_ptr<EFGNode const> parent_;
    shared_ptr<Action> incomingAction_;  // Action performed in the parent node.
    optional<Player> currentPlayer_ = nullopt;
    mutable size_t hashAOH_ = 0;
    int depth_;
};
};  // namespace GTLib2

namespace std { // NOLINT(cert-dcl58-cpp)

using GTLib2::EFGNode;

template<>
struct hash<shared_ptr<EFGNode>> {
    size_t operator()(const shared_ptr<EFGNode> &p) const {
        return p->getHash();
    }
};

template<>
struct equal_to<shared_ptr<EFGNode>> {
    bool operator()(const shared_ptr<EFGNode> &a,
                    const shared_ptr<EFGNode> &b) const {
        return *a == *b;
    }
};

template<>
struct hash<EFGNode *> {
    size_t operator()(const EFGNode *p) const {
        return p->getHash();
    }
};

template<>
struct equal_to<EFGNode *> {
    bool operator()(const EFGNode *a,
                    const EFGNode *b) const {
        return *a == *b;
    }
};
}  // namespace std


namespace GTLib2 {

/**
 * Save EFG tree structure in a cache.
 *
 * Calls from EFGNode::performAction() are not cached, so at each call a copy is created.
 * This structure is a caching wrapper built on top of EFGNode.
 * It also caches the retrieval of (augmented) information sets and the nodes within them.
 *
 * You can extend this cache to save more information needed by your algoritm.
 */
class EFGCache {

    /**
     * Root distribution of the nodes
     */
    EFGNodesDistribution rootNodes_;

    /**
     * Many EFGNodes can belong to many (augmented) infosets.
     *
     * These two fields together represent a bipartite graph.
     */
    unordered_map<shared_ptr<EFGNode>, vector<shared_ptr<AOH>>> node2infosets_;
    unordered_map<shared_ptr<AOH>, vector<shared_ptr<EFGNode>>> infoset2nodes_;

    /**
     * Specify that in a given node, with which action new distribution of nodes can be obtained.
     *
     * Note that if you need to access parent nodes, they are saved in each respective node:
     * EFGNode::getParent
     */
    unordered_map<shared_ptr<EFGNode>, EFGActionNodesDistribution> nodesChildren_;

 public:
    EFGCache(const OutcomeDistribution &rootProbDist);
    EFGCache(const EFGNodesDistribution &rootNodes);

    bool hasChildren(const shared_ptr<EFGNode> &node);

    bool hasChildren(const shared_ptr<EFGNode> &node, const shared_ptr<Action> &action);

    inline bool hasNode(const shared_ptr<EFGNode> &node) {
        return node2infosets_.find(node) != node2infosets_.end();
    }

    inline bool hasInfoset(const shared_ptr<AOH> &augInfoset) {
        return infoset2nodes_.find(augInfoset) != infoset2nodes_.end();
    }

    inline const vector<shared_ptr<EFGNode>> & getNodesFor(const shared_ptr<AOH> &augInfoset) {
        return infoset2nodes_[augInfoset];
    }

    inline const vector<shared_ptr<AOH>> & getInfosetsFor(const shared_ptr<EFGNode> &node) {
        return node2infosets_[node];
    }

    long countAugInfosets() {
        return infoset2nodes_.size();
    }

    /**
     * Find infoset for the supplied node.
     *
     * This is equivalent to searching for augmented infoset with the same player
     * as is the acting player in the specified node.
     *
     * This function cannot be called on terminal nodes, as infosets are not defined there.
     * It also crashes if you ask for infoset for a node which is not saved in this cache.
     */
    inline const shared_ptr<AOH> & getInfosetFor(const shared_ptr<EFGNode> &node) {
        return node2infosets_[node][*node->getCurrentPlayer()];
    }

    /**
     * Find augmented infoset for the supplied node.
     *
     * This function cannot be called on terminal nodes, as infosets are not defined there.
     * It also crashes if you ask for infoset for a node which is not saved in this cache.
     */
    inline const shared_ptr<AOH> &
    getAugInfosetFor(const shared_ptr<EFGNode> &node, Player player) {
        return node2infosets_[node][player];
    }

    /**
     * Retrieve children for the node after following some action.
     *
     * The nodes are saved in the cache along their augmented infoset identification.
     */
    const EFGNodesDistribution &
    getChildrenFor(const shared_ptr<EFGNode> &node, const shared_ptr<Action> &action);

    /**
     * Retrieve children for the node after following any action.
     *
     * The nodes are saved in the cache along their augmented infoset identification.
     */
    const EFGActionNodesDistribution &
    getChildrenFor(const shared_ptr<EFGNode> &node);

    inline const EFGNodesDistribution & getRootNodes() {
        return rootNodes_;
    }

 private:
    void updateInfosets(const shared_ptr<EFGNode> &node);
    void createNode(const shared_ptr<EFGNode> &node);

};
};  // namespace GTLib2


#endif  // BASE_EFG_H_

#pragma clang diagnostic pop
