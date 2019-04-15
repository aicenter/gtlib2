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

#include <algorithm>
#include <sstream>
#include <iterator>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {

EFGNodesDistribution EFGNode::performAction(const shared_ptr<Action> &action) const {
    vector<PlayerAction> actionsToBePerformed(performedActionsInThisRound_);
    actionsToBePerformed.emplace_back(*currentPlayer_, action);

    // todo: add check that the given action can be performed!
    //   maybe do the check only in debug mode to sustain performance?

    EFGNodesDistribution newNodes;
    if (remainingPlayersInTheRound_.size() == 1) {
        std::sort(actionsToBePerformed.begin(), actionsToBePerformed.end(),
                  [](const pair<Player, shared_ptr<Action>> &a,
                     const pair<Player, shared_ptr<Action>> &b) {
                      return (a.first < b.first);
                  });

        for (auto &player : state_->getDomain()->getPlayers()) {
            auto action2 = std::find_if(actionsToBePerformed.begin(), actionsToBePerformed.end(),
                                        [&player](pair<Player, shared_ptr<Action>> const &elem) {
                                            return elem.first == player;
                                        });
            if (action2 == actionsToBePerformed.end()) {
                actionsToBePerformed.emplace(
                    actionsToBePerformed.begin() + player, player, make_shared<Action>(NO_ACTION));
            }
        }

        // Last player in the round. So we proceed to the next state
        auto probDist = state_->performActions(actionsToBePerformed);
        for (auto const&[outcome, prob] : probDist) {

            auto newNode = make_shared<EFGNode>(outcome.state_, shared_from_this(),
                                                outcome.privateObservations_,
                                                outcome.publicObservation_,
                                                outcome.rewards_, prob * natureProbability_,
                                                action, depth_ + 1);
            newNodes.emplace_back(newNode, prob);
        }
    } else {
        auto newNode = make_shared<EFGNode>(shared_from_this(),
                                            actionsToBePerformed, action, depth_);
        newNodes.emplace_back(newNode, 1.0);
    }
    return newNodes;
}

EFGNode::EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                 const vector<shared_ptr<Observation>> &privateObservations,
                 const shared_ptr<Observation> &publicObservation,
                 const vector<double> &rewards,
                 double natureProbability, shared_ptr<Action> incomingAction, int depth) {
    state_ = move(newState);
    privateObservations_ = privateObservations;
    publicObservation_ = publicObservation;
    rewards_ = rewards;
    natureProbability_ = natureProbability;
    depth_ = depth;
    remainingPlayersInTheRound_ = state_->getPlayers();
    std::reverse(remainingPlayersInTheRound_.begin(), remainingPlayersInTheRound_.end());
    if (!remainingPlayersInTheRound_.empty()) {
        currentPlayer_ = remainingPlayersInTheRound_.back();
    } else {
        currentPlayer_ = nullopt;
    }
    parent_ = move(parent);
    incomingAction_ = move(incomingAction);

    generateDescriptor();
    generateHash();
}

EFGNode::EFGNode(shared_ptr<EFGNode const> parent,
                 const vector<PlayerAction> &performedActions,
                 shared_ptr<Action> incomingAction, int depth) {
    state_ = parent->state_;
    privateObservations_ = parent->privateObservations_;
    rewards_ = parent->rewards_;
    natureProbability_ = parent->natureProbability_;
    incomingAction_ = move(incomingAction);
    depth_ = depth;
    performedActionsInThisRound_ = performedActions;
    remainingPlayersInTheRound_ = parent->remainingPlayersInTheRound_;
    remainingPlayersInTheRound_.pop_back();
    if (!remainingPlayersInTheRound_.empty()) {
        currentPlayer_ = remainingPlayersInTheRound_.back();
    } else {
        currentPlayer_ = nullopt;
    }
    parent_ = move(parent);

    generateDescriptor();
    generateHash();
}

unsigned long EFGNode::countAvailableActions() const {
    if (currentPlayer_) {
        return state_->countAvailableActionsFor(*currentPlayer_);
    }
    return 0;
}

vector<shared_ptr<Action>> EFGNode::availableActions() const {
    if (currentPlayer_) {
        return state_->getAvailableActionsFor(*currentPlayer_);
    }
    return vector<shared_ptr<Action>>();
}

shared_ptr<AOH> EFGNode::getAOHInfSet() const {
    if (currentPlayer_) {
        auto aoh = getAOH(*currentPlayer_);
        return make_shared<AOH>(*currentPlayer_, aoh);
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
    if (!parent_) {
        return vector<ActionObservation>{
            make_pair(NO_ACTION, privateObservations_[player]->getId())};
    }
    auto aoh = parent_->getAOH(player);
    if (parent_->depth_ != depth_) {
        auto action = std::find_if(parent_->performedActionsInThisRound_.begin(),
                                   parent_->performedActionsInThisRound_.end(),
                                   [&player](pair<int, shared_ptr<Action>> const &elem) {
                                       return elem.first == player;
                                   });
        if (action != parent_->performedActionsInThisRound_.end()) {
            aoh.emplace_back(action->second->getId(), privateObservations_[player]->getId());
        } else if (*parent_->currentPlayer_ == player) {
            aoh.emplace_back(incomingAction_->getId(), privateObservations_[player]->getId());
        } else {
            aoh.emplace_back(NO_ACTION, privateObservations_[player]->getId());
        }
    }
    return aoh;
}

optional<Player> EFGNode::getCurrentPlayer() const {
    return currentPlayer_;
}

ActionSequence EFGNode::getActionsSeqOfPlayer(Player player) const {
    auto actSeq = parent_ ? parent_->getActionsSeqOfPlayer(player) : ActionSequence();
    if (parent_ && parent_->currentPlayer_ && *parent_->currentPlayer_ == player) {
        actSeq.emplace_back(parent_->getAOHInfSet(), incomingAction_);
    }
    return actSeq;
}

shared_ptr<EFGNode const> EFGNode::getParent() const {
    return parent_;
}

shared_ptr<State> EFGNode::getState() const {
    return state_;
}

bool EFGNode::isContainedInInformationSet(const shared_ptr<AOH> &infSet) const {
    auto mySet = getAOHInfSet();
    if (mySet == nullptr) {
        return false;
    }
    return *mySet == *infSet;
}

const shared_ptr<Action> &EFGNode::getIncomingAction() const {
    return incomingAction_;
}

double EFGNode::getProbabilityOfActionsSeqOfPlayer(Player player,
                                                   const BehavioralStrategy &strat) const {
    if (!parent_) {
        return 1.0;
    }

    auto prob = parent_->getProbabilityOfActionsSeqOfPlayer(player, strat);

    if (*parent_->getCurrentPlayer() == player) {
        auto parentInfSet = parent_->getAOHInfSet();
        auto &actionsProbs = strat.at(parentInfSet);
        double actionProb = (actionsProbs.find(incomingAction_) != actionsProbs.end()) ?
                            actionsProbs.at(incomingAction_) : 0.0;
        return actionProb * prob;
    } else {
        return prob;
    }
}

void EFGNode::generateDescriptor() const {
    if (parent_) {
        descriptor_ = parent_->descriptor_;
        descriptor_.push_back(incomingAction_->getId());
    }
    if (!parent_ || depth_ != parent_->depth_) {
        for (const auto &observation : privateObservations_) {
            descriptor_.push_back(observation->getId());
        }
    }
    if(!isTerminal()) {
        descriptor_.push_back(*currentPlayer_);
    }
}

void EFGNode::generateHash() const {
    hashNode_ = hashWithSeed(descriptor_.data(), descriptor_.size() * sizeof(uint32_t), 1412914847);
}

bool EFGNode::compareAOH(const EFGNode &rhs) const {
    if (parent_) {
        if (depth_ != parent_->depth_) {
            for (int i = 0; i < privateObservations_.size(); ++i) {
                if (privateObservations_[i]->getId() != rhs.privateObservations_[i]->getId()) {
                    return false;
                }
            }
        }
        return *incomingAction_ == *rhs.incomingAction_ && parent_->compareAOH(*rhs.parent_);
    }
    for (int i = 0; i < privateObservations_.size(); ++i) {
        if (privateObservations_[i]->getId() != rhs.privateObservations_[i]->getId()) {
            return false;
        }
    }
    return true;
}

bool EFGNode::operator==(const EFGNode &rhs) const {
    if (hashNode_ != rhs.hashNode_) return false;
    if (descriptor_.size() != rhs.descriptor_.size()) return false;
    return !memcmp(descriptor_.data(), rhs.descriptor_.data(), descriptor_.size());
}

int EFGNode::getDistanceFromRoot() const {
    if (!parent_) return 0;
    return 1 + parent_->getDistanceFromRoot();
}

ObservationId EFGNode::getLastObservationIdOfCurrentPlayer() const {
    return privateObservations_[*currentPlayer_]->getId();
}

string EFGNode::toString() const {
    string s = "Player: " + to_string(*currentPlayer_) + "\nIncoming action: ";
    s += incomingAction_ ? incomingAction_->toString() : "none (root)";
    s += ", nature probability: " + to_string(natureProbability_) + "\n" +
        "State: " + state_->toString() + "\nRewards: [";
    std::stringstream rews;
    std::copy(rewards_.begin(), rewards_.end(), std::ostream_iterator<int>(rews, ", "));
    std::stringstream rem;
    std::copy(remainingPlayersInTheRound_.begin(), remainingPlayersInTheRound_.end(),
              std::ostream_iterator<int>(rem, ", "));
    s += rews.str().substr(0, rews.str().length() - 2) + "]\nObs: [";
    for (auto &i : privateObservations_) {
        s += i->toString() + " ";
    }
    s += "]\nRemaining players: [" + rem.str().substr(0, rem.str().length() - 2)
        + "]\nPerformed actions in this round:\n";
    for (auto &i : performedActionsInThisRound_) {
        s += to_string(i.first) + "  " + i.second->toString() + "\n";
    }
    return s;
}

int EFGNode::getNumberOfRemainingPlayers() const {
    return static_cast<int>(remainingPlayersInTheRound_.size());
}

ObservationId EFGNode::getLastObservationOfPlayer(Player player) const {
    return privateObservations_[player]->getId();
}
int EFGNode::getDepth() const {
    return depth_;
}
ActionId EFGNode::getIncomingActionId() const {
    return incomingAction_->getId();
}
bool EFGNode::noActionPerformedInThisRound() const {
    return performedActionsInThisRound_.empty();
}
bool EFGNode::isTerminal() const {
    return currentPlayer_ == nullopt;
}

}  // namespace GTLib2

#pragma clang diagnostic pop
