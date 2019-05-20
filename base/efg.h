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


enum EFGNodeType {
    ChanceNode,
    PlayerNode,
    TerminalNode
};

class EFGChanceAction: public Action {
 private:
    explicit inline EFGChanceAction(ActionId id, double chanceProb)
        : Action(id), chanceProb_(chanceProb) {}
 public:
    inline string toString() const override {
        return to_string(id_) + ", p=" + to_string(chanceProb_);
    }
    bool operator==(const Action &that) const override;
    inline size_t getHash() const override {
        return id_;
    }

    double chanceProb_ = 1.0;
    friend EFGNode; // only EFGNode can create instance of EFGChanceAction
};


/**
 * Vector of nodes after following a specified action id.
 *
 * Note that action ids are indexed from 0..N-1
 */
typedef vector <shared_ptr<EFGNode>> EFGChildNodes;

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

    explicit EFGNode(EFGNodeType type, shared_ptr<EFGNode const> parent,
                     shared_ptr <Action> incomingAction, shared_ptr <Outcome> lastOutcome,
                     double chanceProb, OutcomeDistribution outcomeDist,
                     vector <Player> remainingRoundPlayers, vector <PlayerAction> roundActions,
                     unsigned int stateDepth);

    inline bool hasNewOutcome() const {
        if(!parent_) return false;
        return parent_->stateDepth_ != stateDepth_;
    }

    /**
     * Returns the sequence of actions performed by the player since the root.
     */
    shared_ptr <ActionSequence> getActionsSeqOfPlayer(Player player) const;

//    double getProbabilityOfActionsSeqOfPlayer(Player player, const BehavioralStrategy &strat) const;

    /**
     * Returns number of available actions for the current player
     * Leaves have zero available actions.
     */
    unsigned long countAvailableActions() const;

    /**
     * Returns available actions for the current player
     * Leaves do not have any available actions.
     */
    vector <shared_ptr<Action>> availableActions() const;

    /**
     * Perform the given action and returns the next node
     * or nodes in case of stochastic games together with the probabilities.
     */
    shared_ptr <EFGNode> performAction(const shared_ptr <Action> &action) const;


    double chanceProbForAction(const shared_ptr <Action> &action) const;
    ProbDistribution chanceProbs() const;

    double getProbabilityOfActionSeq(Player player, const BehavioralStrategy &strat) const;

    /**
     * Gets the augmented information set of the node.
     *
     * Note that augmented information sets coincide with ordinary information sets
     * when the requested player is acting in this node.
     */
    shared_ptr <AOH> getAOHAugInfSet(Player player) const;

    /**
     * Gets the information set of the node represented as ActionObservationHistory set.
     */
    inline shared_ptr <AOH> getAOHInfSet() const {
        assert(type_ == PlayerNode); // TODO not working assert ???
        return getAOHAugInfSet(currentPlayer_);
    }

    vector <ActionObservationIds> getAOids(Player player) const;

    /**
     * Gets the public state of the node based on public observation history.
     */
    shared_ptr <EFGPublicState> getPublicState() const;

    inline HashType getHash() const { return hashNode_; };
    string toString() const;

    /**
     * Get current player playing in this round.
     */
    Player getPlayer() const;

    vector<double> getUtilities() const;

    bool operator==(const EFGNode &rhs) const;

    const shared_ptr<EFGNode const> parent_;
    const EFGNodeType type_;
    const double chanceReachProb_;
    const double lastChanceProb_;
    const vector <ActionId> history_;
    const shared_ptr <Action> incomingAction_;
    const int stateDepth_;
    const int efgDepth_;

 private:
    shared_ptr <EFGNode> performChanceAction(const shared_ptr <Action> &action) const;
    shared_ptr <EFGNode> performPlayerAction(const shared_ptr <Action> &action) const;
    shared_ptr <EFGNode> createNodeForSpecificOutcome(
        const shared_ptr <Action> &playerAction, const OutcomeEntry &specificOutcome) const;
    vector <shared_ptr<Action>> createChanceActions() const;

    const Player currentPlayer_;
    const shared_ptr <Outcome> lastOutcome_;
    const OutcomeDistribution outcomeDist_;
    const vector <Player> remainingRoundPlayers_;
    const vector <PlayerAction> roundActions_;
    const vector<double> cumRewards_;

    const HashType hashNode_ = 0;

    friend bool isEFGNodeAndStateConsistent(const Domain &domain);
    friend bool isNumPlayersCountActionsConsistentInState(const Domain &domain);
};

class EFGPublicState {
 public:
    EFGPublicState(const shared_ptr <Observation> &publicObservation);
    EFGPublicState(const shared_ptr <EFGPublicState> &parent,
                   const shared_ptr <Observation> &publicObservation);

    inline const vector <shared_ptr<Observation>> &getPublicHistory() const {
        return publicObsHistory_;
    }
    inline HashType getHash() const {
        return hashNode_;
    };
    bool operator==(const EFGPublicState &rhs) const;

    inline const vector <uint32_t> &getDescriptor() const {
        return descriptor_;
    }

 private:
    vector <shared_ptr<Observation>> publicObsHistory_;
    mutable HashType hashNode_ = 0;
    mutable vector <uint32_t> descriptor_;

    void generateDescriptor() const;
    void generateHash() const;

};

shared_ptr<EFGNode> createRootEFGNode(const OutcomeDistribution &rootOutcomes);
inline shared_ptr<EFGNode> createRootEFGNode(const Domain &domain) {
    return createRootEFGNode(domain.getRootStatesDistribution());
}

};  // namespace GTLib2

MAKE_EQ(GTLib2::EFGNode)
MAKE_EQ(GTLib2::EFGPublicState)

MAKE_HASHABLE(GTLib2::EFGNode)
MAKE_HASHABLE(GTLib2::EFGPublicState)

#endif  // BASE_EFG_H_

#pragma clang diagnostic pop
