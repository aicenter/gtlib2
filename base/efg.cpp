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


#include "base/efg.h"
#include "algorithms/common.h"
#include "efg.h"


#include <algorithm>
#include <sstream>
#include <iterator>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {

EFGNodesDistribution EFGNode::performAction(const shared_ptr<Action> &action) const {
    vector<PlayerAction> actionsToBePerformed(performedActionsInThisRound);
    actionsToBePerformed.emplace_back(*currentPlayer, action);

    // todo: add check that the given action can be performed!
    //   maybe do the check only in debug mode to sustain performance?

    EFGNodesDistribution newNodes;
    if (remainingPlayersInTheRound.size() == 1) {
        std::sort(actionsToBePerformed.begin(), actionsToBePerformed.end(),
                  [](const pair<int, shared_ptr<Action>> &a,
                     const pair<int, shared_ptr<Action>> &b) {
                      return (a.first < b.first);
                  });

        for (auto &i : state->getDomain()->getPlayers()) {
            auto action2 = std::find_if(actionsToBePerformed.begin(), actionsToBePerformed.end(),
                                        [&i](pair<int, shared_ptr<Action>> const &elem) {
                                            return elem.first == i;
                                        });
            if (action2 == actionsToBePerformed.end()) {
                actionsToBePerformed.emplace(
                    actionsToBePerformed.begin() + i, i, make_shared<Action>(NO_ACTION));
            }
        }

        // Last player in the round. So we proceed to the next state
        auto probDist = state->performActions(actionsToBePerformed);
        for (auto const&[outcome, prob] : probDist) {  // works in GCC 7.3

            auto newNode = make_shared<EFGNode>(outcome.state, shared_from_this(),
                                                outcome.observations, outcome.rewards,
                                                prob * natureProbability, action, depth + 1);
            newNodes.emplace_back(newNode, prob);
        }
    } else {
        auto
            newNode = make_shared<EFGNode>(shared_from_this(), actionsToBePerformed, action, depth);
        newNodes.emplace_back(newNode, 1.0);
    }
    return newNodes;
}

EFGNode::EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                 const vector<shared_ptr<Observation>> &observations,
                 const vector<double> &rewards,
                 double natureProbability, shared_ptr<Action> incomingAction, int depth) {
    this->state = move(newState);
    this->observations = observations;
    this->rewards = rewards;
    this->natureProbability = natureProbability;
    this->depth = depth;
    remainingPlayersInTheRound = state->getPlayers();
    std::reverse(remainingPlayersInTheRound.begin(), remainingPlayersInTheRound.end());
    if (!remainingPlayersInTheRound.empty()) {
        currentPlayer = remainingPlayersInTheRound.back();
    } else {
        currentPlayer = nullopt;
    }
    this->parent = move(parent);
    this->incomingAction = move(incomingAction);
}

EFGNode::EFGNode(shared_ptr<EFGNode const> parent,
                 const vector<PlayerAction> &performedActions,
                 shared_ptr<Action> incomingAction, int depth) {
    this->state = parent->state;
    this->observations = parent->observations;
    this->rewards = parent->rewards;
    this->natureProbability = parent->natureProbability;
    this->incomingAction = move(incomingAction);
    this->depth = depth;
    this->performedActionsInThisRound = performedActions;
    remainingPlayersInTheRound = parent->remainingPlayersInTheRound;
    remainingPlayersInTheRound.pop_back();
    if (!remainingPlayersInTheRound.empty()) {
        currentPlayer = remainingPlayersInTheRound.back();
    } else {
        currentPlayer = nullopt;
    }
    this->parent = move(parent);
}

vector<shared_ptr<Action>> EFGNode::availableActions() const {
    if (currentPlayer) {
        return state->getAvailableActionsFor(*currentPlayer);
    }
    return vector<shared_ptr<Action>>();
}

shared_ptr<AOH> EFGNode::getAOHInfSet() const {
    if (currentPlayer) {
        auto aoh = getAOH(*currentPlayer);
        return make_shared<AOH>(*currentPlayer, aoh);
    } else {
        // todo: shouldn't the return type be optional??
        return nullptr;
    }
}

shared_ptr<AOH> EFGNode::getAOHAugInfSet(Player player) const {
    auto aoh = getAOH(player);
    return make_shared<AOH>(player, aoh);
}

vector<ActionObservation> EFGNode::getAOH(Player player) const {
    if (!parent) {
        return vector<ActionObservation>{
            std::make_pair(NO_ACTION, this->observations[player]->getId())};
    }
    auto aoh = this->parent->getAOH(player);
    if (parent->depth != depth) {
        auto action = std::find_if(parent->performedActionsInThisRound.begin(),
                                   parent->performedActionsInThisRound.end(),
                                   [&player](pair<int, shared_ptr<Action>> const &elem) {
                                       return elem.first == player;
                                   });
        if (action != parent->performedActionsInThisRound.end()) {
            aoh.emplace_back(action->second->getId(), observations[player]->getId());
        } else if (*parent->currentPlayer == player) {
            aoh.emplace_back(incomingAction->getId(), observations[player]->getId());
        } else {
            aoh.emplace_back(NO_ACTION, observations[player]->getId());
        }
    }
    return aoh;
}

optional<Player> EFGNode::getCurrentPlayer() const {
    return currentPlayer;
}

ActionSequence EFGNode::getActionsSeqOfPlayer(int player) const {
    auto actSeq = this->parent ? this->parent->getActionsSeqOfPlayer(player) : ActionSequence();
    if (parent && parent->currentPlayer && *parent->currentPlayer == player) {
        actSeq.emplace_back(parent->getAOHInfSet(), incomingAction);
    }
    return actSeq;
}

shared_ptr<EFGNode const> EFGNode::getParent() const {
    return parent;
}

shared_ptr<State> EFGNode::getState() const {
    return state;
}

bool EFGNode::isContainedInInformationSet(const shared_ptr<AOH> &infSet) const {
    auto mySet = this->getAOHInfSet();
    if (mySet == nullptr) {
        return false;
    }
    return *mySet == *infSet;
}

const shared_ptr<Action> &EFGNode::getIncomingAction() const {
    return incomingAction;
}

double EFGNode::getProbabilityOfActionsSeqOfPlayer(
    int player, const BehavioralStrategy &strat) const {
    if (!parent) {
        return 1.0;
    }

    auto prob = parent->getProbabilityOfActionsSeqOfPlayer(player, strat);

    if (*parent->getCurrentPlayer() == player) {
        auto parentInfSet = parent->getAOHInfSet();
        auto &actionsProbs = strat.at(parentInfSet);
        double actionProb = (actionsProbs.find(incomingAction) != actionsProbs.end()) ?
                            actionsProbs.at(incomingAction) : 0.0;
        return actionProb * prob;
    } else {
        return prob;
    }
}

size_t EFGNode::getHashedAOHs() const {
    if (hashAOH > 0) {
        return hashAOH;
    }
    if (parent) {
        hashAOH = parent->getHashedAOHs();
        boost::hash_combine(hashAOH, incomingAction->getId());
    }
    if (!parent || depth != parent->depth) {
        for (auto &i : observations) {
            boost::hash_combine(hashAOH, i->getId());
        }
    }
    return hashAOH;
}

size_t EFGNode::getHash() const {
    auto seed = getHashedAOHs();
    boost::hash_combine(seed, performedActionsInThisRound.size());
    boost::hash_combine(seed, remainingPlayersInTheRound.size());
    return seed;
}

bool EFGNode::compareAOH(const EFGNode &rhs) const {
    if (this->parent) {
        if (depth != parent->depth) {
            for (int i = 0; i < observations.size(); ++i) {
                if (observations[i]->getId() != rhs.observations[i]->getId()) {
                    return false;
                }
            }
        }
        return *incomingAction == *rhs.incomingAction && parent->compareAOH(*rhs.parent);
    }
    for (int i = 0; i < observations.size(); ++i) {
        if (observations[i]->getId() != rhs.observations[i]->getId()) {
            return false;
        }
    }
    return true;
}

bool EFGNode::operator==(const EFGNode &rhs) const {
    if (this->performedActionsInThisRound.size() != rhs.performedActionsInThisRound.size() ||
        this->observations.size() != rhs.observations.size()) {
        return false;
    }
    for (auto const&[player, action] : this->performedActionsInThisRound) {  // works in GCC 7.3
        auto action2 =
            std::find_if(rhs.performedActionsInThisRound.begin(),
                         rhs.performedActionsInThisRound.end(),
                         [&player](pair<int, shared_ptr<Action>> const &elem) {
                             return elem.first == player;
                         });
        if (action2 == rhs.performedActionsInThisRound.end()
            || !(*action2->second == *action)) {
            return false;
        }
    }
    return depth == rhs.depth && hashAOH == rhs.hashAOH
        && this->remainingPlayersInTheRound == rhs.remainingPlayersInTheRound && compareAOH(rhs);
}

int EFGNode::getDistanceFromRoot() const {
    if (!parent)
        return 0;
    return 1 + parent->getDistanceFromRoot();
}

ObservationId EFGNode::getLastObservationIdOfCurrentPlayer() const {
    return observations[*currentPlayer]->getId();
}

string EFGNode::toString() const {
    string s = "Player: " + to_string(*currentPlayer) + ", incoming action: ";
    s += incomingAction ? incomingAction->toString() : "none (root)";
    s += ", nature probability: " + to_string(natureProbability) + "\n" +
        state->toString() + "\nRewards: [";
    std::stringstream rews;
    std::copy(rewards.begin(), rewards.end(), std::ostream_iterator<int>(rews, ", "));
    std::stringstream rem;
    std::copy(remainingPlayersInTheRound.begin(), remainingPlayersInTheRound.end(),
              std::ostream_iterator<int>(rem, ", "));
    s += rews.str().substr(0, rews.str().length() - 2) + "]\nObs: [";
    for (auto &i : observations) {
        s += i->toString() + " ";
    }
    s += "]\nRemaining players: [" + rem.str().substr(0, rem.str().length() - 2)
        + "]\nPerformed actions in this round:\n";
    for (auto &i : performedActionsInThisRound) {
        s += to_string(i.first) + "  " + i.second->toString() + "\n";
    }
    s += "\n";
    return s;
}

int EFGNode::getNumberOfRemainingPlayers() const {
    return static_cast<int>(remainingPlayersInTheRound.size());
}

ObservationId EFGNode::getLastObservationOfPlayer(Player player) const {
    return observations[player]->getId();
}
int EFGNode::getDepth() const {
    return depth;
}
ActionId EFGNode::getIncomingActionId() const {
    return incomingAction->getId();
}
bool EFGNode::noActionPerformedInThisRound() const {
    return performedActionsInThisRound.empty();
}
bool EFGNode::isTerminal() const {
    return currentPlayer == nullopt;
}


EFGCache::EFGCache(const GTLib2::EFGNodesDistribution &rootNodesDist) {
    rootNodes_ = rootNodesDist;
    for (auto &[node, _]: rootNodesDist) {
        createNode(node);
        updateInfosets(node);
    }
}

EFGCache::EFGCache(const OutcomeDistribution &rootProbDist)
    : EFGCache(algorithms::createRootEFGNodesFromInitialOutcomeDistribution(rootProbDist)) {
}

const EFGNodesDistribution &
EFGCache::getChildrenFor(const shared_ptr<EFGNode> &node, const shared_ptr<Action> &action) {
    auto maybeNode = nodesChildren_.find(node);
    if (maybeNode == nodesChildren_.end()) {
        // Node not found -- maybe trying to get children
        // for a node gotten outside from cache?
        assert(false);
    }

    // fetch from cache if possible
    auto & nodeDist = maybeNode->second;
    auto actionDist = nodeDist.find(action);
    if (actionDist != nodeDist.end()) {
        return actionDist->second;
    }

    // create new nodes and save them to cache
    auto newDist = node->performAction(action);
    nodeDist.insert(std::make_pair(action, newDist));

    for (auto &[childNode, _]: newDist) {
        createNode(childNode);
        updateInfosets(childNode);
    }

    // retrieve from map directly to return a reference,
    // this is guaranteed to exist there since we've just inserted it
    return nodesChildren_
        .find(node)->second
        .find(action)->second;
}

void EFGCache::updateInfosets(const shared_ptr<EFGNode> &node) {
    vector<shared_ptr<AOH>> infosets;

    for (Player pl: node->getState()->getPlayers()) {
        auto infoset = node->getAOHAugInfSet(pl);
        auto maybe_infoset = infoset2nodes_.find(infoset);

        if (maybe_infoset == infoset2nodes_.end()) {
            // infoset not found yet, initialize it with this node
            infosets.emplace_back(infoset);
            infoset2nodes_.emplace(infoset, vector<shared_ptr<EFGNode>>{node});
        } else {
            // infoset found, append this node
            infoset = maybe_infoset->first;
            infosets.emplace_back(infoset);
            maybe_infoset->second.push_back(node);
        }
    }
    node2infosets_.emplace(node, infosets);
}

void EFGCache::createNode(const shared_ptr<EFGNode> &node) {
    nodesChildren_.emplace(node, EFGActionNodesDistribution());
}

}  // namespace GTLib2

#pragma clang diagnostic pop
