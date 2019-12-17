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


#ifndef BASE_FOGEFG_H_
#define BASE_FOGEFG_H_

#include "base/efg.h"

namespace GTLib2 {

/**
 * EFGNode that is implemented on top of State domain representation
 * (FOG, factored observations game).
 *
 * Note that **many calls are not cached!** Use EFGCache to save the tree structure,
 * and iterate over the tree using the EFGCache::getChildrenFor() method.
 */
class FOG2EFGNode: public Node<FOG2EFGNode>,
                   public EFGNode,
                   public std::enable_shared_from_this<FOG2EFGNode const> {
 public:

    explicit FOG2EFGNode(const shared_ptr<FOG2EFGNode const> &parent,
                         shared_ptr<Action> incomingAction, EFGNodeType type,
                         shared_ptr<Outcome> lastOutcome,
                         OutcomeDistribution outcomeDist,
                         vector<Player> remainingRoundPlayers,
                         vector<PlayerAction> roundActions,
                         unsigned int stateDepth);

    inline FOG2EFGNode(const FOG2EFGNode &other) :
        Node<FOG2EFGNode>(other), EFGNode(other),
        std::enable_shared_from_this<FOG2EFGNode const>(other),
        stateDepth_(other.stateDepth_), incomingAction_(other.incomingAction_),
        lastOutcome_(other.lastOutcome_), outcomeDist_(other.outcomeDist_),
        remainingRoundPlayers_(other.remainingRoundPlayers_), roundActions_(other.roundActions_),
        cumRewards_(other.cumRewards_) {}

    // EFGNode
    inline EFGNodeSpecialization getSpecialization() const override { return NoSpecialization; }
    inline shared_ptr<EFGNode const> getParent() const override { return parent_; }
    unsigned int efgDepth() const override { return history_.size(); };
    unsigned long countAvailableActions() const override;
    vector<shared_ptr<Action>> availableActions() const override;
    shared_ptr<Action> getActionByID(ActionId id) const override;
    shared_ptr<EFGNode> performAction(const shared_ptr<Action> &action) const override;
    double chanceProbForAction(const ActionId &action) const override;
    double chanceProbForAction(const shared_ptr<Action> &action) const override;
    ProbDistribution chanceProbs() const override;
    vector<ActionObservationIds> getAOids(Player player) const override;
    vector<ObservationId> getPubObsIds() const override;
    double getProbabilityOfActionSeq(Player player,
                                     const BehavioralStrategy &strat) const override; // todo: remove
    shared_ptr<ActionSequence> getActionsSeqOfPlayer(Player player) const override; // todo: remove
    inline const vector<ActionId> &getHistory() const override { return history_; }
    inline HashType getHash() const override { return Node::getHash(); }

    // FOG
    const shared_ptr<FOG2EFGNode> getChildAt(EdgeId index) const;
    const shared_ptr<State> getState() const { return lastOutcome_->state; }
    inline unsigned int stateDepth() const { return stateDepth_; }
    inline bool operator==(const FOG2EFGNode &rhs) const { return Node::operator==(rhs); }

    inline void describeNewOutcome() {
        if(hasNewOutcome()) {
            if (lastOutcome_->publicObservation != nullptr) {LOG_INFO("New public  obs: " << *lastOutcome_->publicObservation)}
            else {LOG_INFO("No public  obs")}
            LOG_INFO("New private obs for Pl0: " << *lastOutcome_->privateObservations.at(0))
            LOG_INFO("New private obs for Pl1: " << *lastOutcome_->privateObservations.at(1))
            LOG_INFO("New rewards: " << lastOutcome_->rewards)
        }
    }

 private:
    shared_ptr<FOG2EFGNode> performChanceAction(const shared_ptr<Action> &action) const;
    shared_ptr<FOG2EFGNode> performPlayerAction(const shared_ptr<Action> &action) const;
    shared_ptr<FOG2EFGNode> createNodeForSpecificOutcome(const shared_ptr<Action> &playerAction,
                                                         const OutcomeEntry &specificOutcome) const;
    vector<shared_ptr<Action>> createChanceActions() const;

    inline vector<double> getNewCumRewards(const vector<double> &newRewards) const {
        return hasNewOutcome() ? parent_->cumRewards_ + newRewards
                               : parent_->cumRewards_;
    }

    /**
     * Specify if this node has received a new outcome, i.e. it is a result of transition
     * in the underlying POSG graph.
     */
    inline bool hasNewOutcome() const {
        if (parent_ == nullptr) return false;
        return parent_->stateDepth_ != stateDepth_;
    }

    const unsigned int stateDepth_;
    const shared_ptr<Action> incomingAction_;
    const shared_ptr<Outcome> lastOutcome_;
    const OutcomeDistribution outcomeDist_;
    const vector<Player> remainingRoundPlayers_;
    const vector<PlayerAction> roundActions_;
    const vector<double> cumRewards_;
};

shared_ptr<EFGNode> createRootEFGNode(const OutcomeDistribution &rootOutcomes);
inline shared_ptr<EFGNode> createRootEFGNode(const Domain &domain) {
    return createRootEFGNode(domain.getRootStatesDistribution());
}

typedef function<void(shared_ptr<EFGNode>)> EFGNodeCallback;

template<>
inline unsigned int nodeChildCnt<EFGNode>(const shared_ptr<EFGNode> &node) {
    return node->countAvailableActions();
}
template<>
inline shared_ptr<EFGNode>
nodeChildExpander<EFGNode>(const shared_ptr<EFGNode> &node, EdgeId index) {
    return node->performAction(node->getActionByID(index));
}
template<>
inline unsigned int nodeChildCnt<FOG2EFGNode>(const shared_ptr<FOG2EFGNode> &node) {
    return node->countAvailableActions();
}
template<>
inline shared_ptr<FOG2EFGNode>
nodeChildExpander<FOG2EFGNode>(const shared_ptr<FOG2EFGNode> &node, EdgeId index) {
    return node->getChildAt(index);
}

/**
 * Call supplied function at each EFGNode of the EFG tree, including leaves.
 * The tree is walked as DFS up to maximum depth allowed by the domain.
 */
inline void treeWalk(const Domain &domain, const NodeCallback<FOG2EFGNode> &function) {
    auto rootNode = dynamic_pointer_cast<FOG2EFGNode>(createRootEFGNode(domain));
    treeWalk(rootNode, function);
}

}

MAKE_EQ(GTLib2::FOG2EFGNode)
MAKE_HASHABLE(GTLib2::FOG2EFGNode)

#endif  // BASE_EFG_H_
