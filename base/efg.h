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

#include "base/base.h"
#include "base/hashing.h"


namespace GTLib2 {

class EFGNode;
class EFGPublicState;

/**
 * The chance probability of an EFGNode after performing some action,
 * i.e. not from the root, but from a parent node `h` to it's child `g` -- $\pi_c(g|h)$.
 */
// todo: refactor pair into a more readable struct (avoid using .first/.second)
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
            const vector<shared_ptr<Observation>> &privateObservations,
            const shared_ptr<Observation> &publicObservation,
            const vector<double> &rewards,
            double natureProbability, shared_ptr<Action> incomingAction, int depth);

    /**
     * Returns the sequence of actions performed by the player since the root.
     */
    ActionSequence getActionsSeqOfPlayer(Player player) const;

    double getProbabilityOfActionsSeqOfPlayer(Player player, const BehavioralStrategy &strat) const;

    /**
     * Returns number of available actions for the current player
     * Leaves have zero available actions.
     */
    unsigned long countAvailableActions() const;

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
     * Gets the public state of the node based on public observation history.
     */
    shared_ptr<EFGPublicState> getPublicState() const;

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

    inline HashType getHash() const {
        return hashNode_;
    };

    /**
     * Get the depth in the sense of State depth
     */
    int getStateDepth() const;

    /**
     * Get the depth in the sense of EFG depth
     */
    int getEFGDepth() const;

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

    /**
     * Cumulative rewards we have gotten on the path from the root to this node
     */
    vector<double> rewards_;

    /**
     * Nature reach probability from root
     */
    double natureProbability_;

    bool operator==(const EFGNode &rhs) const;

    inline const vector<uint32_t>& getDescriptor() const {
        return descriptor_;
    }

 private:
    vector<ActionObservation> getAOH(Player player) const;
    bool compareAOH(const EFGNode &rhs) const;

    void generateDescriptor() const;
    void generateHash() const;

    vector<shared_ptr<Observation>> privateObservations_;
    shared_ptr<Observation> publicObservation_;
    vector<PlayerAction> performedActionsInThisRound_;
    vector<Player> remainingPlayersInTheRound_;
    // todo: const for member variables and constructors?
    shared_ptr<State> state_;
    shared_ptr<EFGNode const> parent_;
    shared_ptr<Action> incomingAction_;  // Action performed in the parent node.
    optional<Player> currentPlayer_ = nullopt;
    int stateDepth_;

    mutable HashType hashNode_ = 0;
    mutable vector<uint32_t> descriptor_;
};

class EFGPublicState {
 public:
    EFGPublicState(const shared_ptr<Observation> &publicObservation);
    EFGPublicState(const shared_ptr<EFGPublicState>& parent, const shared_ptr<Observation> &publicObservation);

    inline const vector<shared_ptr<Observation>>& getPublicHistory() const {
        return publicObsHistory_;
    }
    inline HashType getHash() const {
        return hashNode_;
    };
    bool operator==(const EFGPublicState &rhs) const;

    inline const vector<uint32_t>& getDescriptor() const {
        return descriptor_;
    }

 private:
    vector<shared_ptr<Observation>> publicObsHistory_;
    mutable HashType hashNode_ = 0;
    mutable vector<uint32_t> descriptor_;

    void generateDescriptor() const;
    void generateHash() const;

};

};  // namespace GTLib2

MAKE_PTR_HASHABLE(shared_ptr<GTLib2::EFGNode>)
MAKE_PTR_HASHABLE(GTLib2::EFGNode*)
MAKE_PTR_HASHABLE(shared_ptr<GTLib2::EFGPublicState>)
MAKE_PTR_HASHABLE(GTLib2::EFGPublicState*)

MAKE_PTR_EQ(shared_ptr<GTLib2::EFGNode>)
MAKE_PTR_EQ(GTLib2::EFGNode*)
MAKE_PTR_EQ(shared_ptr<GTLib2::EFGPublicState>)
MAKE_PTR_EQ(GTLib2::EFGPublicState*)

#endif  // BASE_EFG_H_

#pragma clang diagnostic pop
