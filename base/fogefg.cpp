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


#include "base/fogefg.h"
#include "algorithms/common.h"

namespace GTLib2 {

FOG2EFGNode::FOG2EFGNode(const shared_ptr<FOG2EFGNode const> &parent,
                         shared_ptr<Action> incomingAction,
                         EFGNodeType type,
                         shared_ptr<Outcome> lastOutcome,
                         OutcomeDistribution outcomeDist,
                         vector<Player> remainingRoundPlayers,
                         vector<PlayerAction> roundActions,
                         unsigned int stateDepth) :
    Node<FOG2EFGNode>(
        parent,
        parent == nullptr ? nullopt : optional(incomingAction->getId())), // can't call isRoot here
    EFGNode(type,
            type == PlayerNode ? remainingRoundPlayers[0] : NO_PLAYER,
            type == TerminalNode ? parent_->cumRewards_
                + lastOutcome->rewards // terminals always make state transition
                                 : vector<double>()),
    stateDepth_(stateDepth),
    incomingAction_(move(incomingAction)),
    lastOutcome_(move(lastOutcome)),
    outcomeDist_(move(outcomeDist)),
    remainingRoundPlayers_(move(remainingRoundPlayers)),
    roundActions_(move(roundActions)),
    cumRewards_(isRoot() ? vector<double>(2, 0.0) : getNewCumRewards(lastOutcome_->rewards)) {

#ifndef NDEBUG // equivalent to assert
    // state terminal implies EFGNode terminal
    assert(!parent_ || (!lastOutcome_->state->isTerminal() || type_ == TerminalNode));

    if (!outcomeDist_.empty()) {
        double sum = 0.;
        for (const auto&[_, prob] : outcomeDist_) {
            sum += prob;
        }
        assert(sum > (1 - 1e-6));
        assert(sum < (1 + 1e-6));
    }
#endif
}

shared_ptr<EFGNode> FOG2EFGNode::performAction(const shared_ptr<Action> &action) const {
    switch (type_) { // dispatch
        case ChanceNode:
            return performChanceAction(action);
        case PlayerNode:
            return performPlayerAction(action);
        case TerminalNode:
            unreachable("Cannot perform any actions in terminal node!");
        default:
            unreachable("unrecognized option!");
    }
}

shared_ptr<FOG2EFGNode> FOG2EFGNode::performChanceAction(const shared_ptr<Action> &action) const {
    // Chance player is always the last player in a given round
    assert(remainingRoundPlayers_.empty());

    // There are two cases where we encounter chance node:
    //
    // 1) Chance is the only EFGNode in the current round
    //     Because: All the players have played NO_ACTION (domain wants to do some kind of padding).
    //              Or it is the initial distribution.
    // 2) Chance comes after all of the round players have played.
    //
    // In both of them, we have already received the outcome distribution by which to play.

    assert(action->getId() >= 0
               && action->getId() < outcomeDist_.size()
               && typeid(*action) == typeid(EFGChanceAction));

    return createNodeForSpecificOutcome(action, outcomeDist_[action->getId()]);
}

shared_ptr<FOG2EFGNode> FOG2EFGNode::performPlayerAction(const shared_ptr<Action> &action) const {
    // When doing player actions, there always must be someone to play.
    assert(!remainingRoundPlayers_.empty());

    auto updatedActions = extend(roundActions_, PlayerAction{currentPlayer_, action});

    // When there are multiple players, we just pass along the data.
    if (remainingRoundPlayers_.size() > 1) {
        const auto shiftedPlayers = vector<Player>(remainingRoundPlayers_.begin() + 1,
                                                   remainingRoundPlayers_.end());
        return make_shared<FOG2EFGNode>(shared_from_this(), action, PlayerNode, lastOutcome_,
                                        OutcomeDistribution(), shiftedPlayers, updatedActions,
                                        stateDepth_);
    }

    // Now we have all the actions of round players that are needed to go to next state.
    const auto &currentState = lastOutcome_->state;
    auto outcomeDistribution = currentState->performPartialActions(updatedActions);
    assert(outcomeDistribution.size() >= 1);

    // Should we create a chance node?
    if (outcomeDistribution.size() > 1) {
        return make_shared<FOG2EFGNode>(shared_from_this(), action, ChanceNode, lastOutcome_,
                                        outcomeDistribution, vector<Player>(), updatedActions,
                                        stateDepth_);
    }

    // There is only one outcome.
    assert(outcomeDistribution[0].prob == 1.0);
    return createNodeForSpecificOutcome(action, outcomeDistribution[0]);
}


shared_ptr<FOG2EFGNode> FOG2EFGNode::createNodeForSpecificOutcome(
    const shared_ptr<Action> &playerAction, const OutcomeEntry &specificOutcome) const {

    // Now there is only one outcome, thus entering a new round. We will create either:
    //
    // - Terminal node - state is terminal, or we run out of state depth
    // - Player node   - there needs to be at least one player playing in next round
    // - Chance node   - there are no players playing (they use NO_ACTION).
    //                   If there is only one chance outcome for the new chance node,
    //                   we will create it anyway, as we need to make sure we store intermediate
    //                   outcomes somewhere and don't have to accumulate them in some while loop
    //                   and pass as a vector.
    const auto &[outcome, chanceProb] = specificOutcome;

    const shared_ptr<State> &nextState = outcome.state;
    const Domain *domain = nextState->getDomain();
    EFGNodeType childType = ChanceNode;
    if (nextState->isTerminal()) {
        childType = TerminalNode;
    } else if (stateDepth_ + 1 == domain->getMaxStateDepth()) {
        childType = TerminalNode;
    } else if (!nextState->getPlayers().empty()) {
        childType = PlayerNode;
    }

    if (childType != ChanceNode) {
        // Enter new round without chance node.
        return make_shared<FOG2EFGNode>(shared_from_this(), playerAction,
                                        childType, make_shared<Outcome>(outcome),
                                        OutcomeDistribution(), nextState->getPlayers(),
                                        vector<PlayerAction>(), stateDepth_ + 1);
    }

    // We will return chance node, but we need to call NO_ACTION
    // for all players to get distribution for the chance player.
    const auto outcomeDistribution = nextState->performPartialActions(vector<PlayerAction>());

    return make_shared<FOG2EFGNode>(shared_from_this(), playerAction, ChanceNode,
                                    make_shared<Outcome>(outcome), outcomeDistribution,
                                    vector<Player>(), vector<PlayerAction>(), stateDepth_ + 1);
}

unsigned long FOG2EFGNode::countAvailableActions() const {
    switch (type_) {
        case PlayerNode:
            return lastOutcome_->state->countAvailableActionsFor(currentPlayer_);
        case ChanceNode:
            return outcomeDist_.size();
        case TerminalNode:
            return 0;
        default:
            unreachable("unrecognized option!");
    }
}

vector<shared_ptr<Action>> FOG2EFGNode::availableActions() const {
    switch (type_) {
        case PlayerNode:
            return lastOutcome_->state->getAvailableActionsFor(currentPlayer_);
        case ChanceNode:
            return createChanceActions();
        case TerminalNode:
            unreachable("Not defined for terminal nodes!");
        default:
            unreachable("unrecognized option!");
    }
}

vector<shared_ptr<Action>> FOG2EFGNode::createChanceActions() const {
    vector<shared_ptr<Action>> actions;
    for (int i = 0; i < outcomeDist_.size(); ++i) {
        actions.emplace_back(make_shared<EFGChanceAction>(i, outcomeDist_[i].prob));
    }
    return actions;
}

double FOG2EFGNode::chanceProbForAction(const ActionId &action) const {
    return outcomeDist_[action].prob;
}

double FOG2EFGNode::chanceProbForAction(const shared_ptr<Action> &action) const {
    assert(type_ == ChanceNode);
    assert(typeid(*action) == typeid(EFGChanceAction));
    return outcomeDist_[action->getId()].prob;
}

ProbDistribution FOG2EFGNode::chanceProbs() const {
    assert(type_ == ChanceNode);
    auto dist = ProbDistribution();
    dist.reserve(outcomeDist_.size());
    for (const auto &outcome : outcomeDist_) {
        dist.push_back(outcome.prob);
    }
    return dist;
}

vector<ActionObservationIds> FOG2EFGNode::getAOids(Player player) const {
    if (!parent_) {
        switch (type_) {
            case ChanceNode:
                // In a root chance node, nobody knows anything yet.
                return {NO_ACTION_OBSERVATION};
            case PlayerNode: {
                // There was only one chance outcome and that's why player node was constructed
                // as a root node. However, the opponent might have learned something, which
                // will be even more refined after the player plays an action, and this is the
                // reason to add observations here as well, and not just go
                // with the NO_ACTION_OBSERVATION.
                //
                // The second observation of which player makes the initial move must be here
                // to ensure that when that player makes an action, it will not be written
                // into the one at index 0, because he didn't make that action when he received
                // the first observation.
                const ObservationId initialObs = lastOutcome_->privateObservations[player]->getId();
                if (getPlayer() == player)
                    return {
                        ActionObservationIds{NO_ACTION, initialObs},
                        ActionObservationIds{NO_ACTION, observationPlayerMove(getPlayer())}
                    };
                else
                    return {ActionObservationIds{NO_ACTION, initialObs}};
            }
            case TerminalNode:
                unreachable("parent cannot be terminal!");
            default:
                unreachable("unrecognized option");
        }
    }

    const ObservationId lastObservation = lastOutcome_->privateObservations[player]->getId();
    const ActionId lastAction = incomingAction_->getId();
    auto aoh = parent_->getAOids(player);

    switch (parent_->type_) {

        case ChanceNode:
            // After chance node, there is always a new round, so we have new private observations.
            // However those can be of value NO_OBSERVATION.

            if (!parent_->parent_ || parent_->lastOutcome_->state->isPlayerMakingMove(player)) {
                // Update observation either
                // - for the root (initial) chance outcomes
                // - if the player was making a move in the round
                aoh[aoh.size() - 1].observation = lastObservation;
            } else {
                // Otherwise append observation, and the player didn't do anything
                // to get this observation (it was announced)
                aoh.emplace_back(ActionObservationIds{NO_ACTION, lastObservation});
            }
            break;

        case PlayerNode:
            // There are two cases of player node:
            // - one that continues to the same round
            // - one that continues to the next round
            //   (because chance node is omitted - there was only one chance action,
            //    so we skip such a node)

            if (parent_->getPlayer() == player) {
                // Give player the action that he actually made.
                aoh[aoh.size() - 1].action = lastAction;
            } else {
                // It was opponent's node, so the player didn't do anything,
                // and didn't get an observation.
                aoh.emplace_back(NO_ACTION_OBSERVATION);
            }

            // If it is a new round, update the observation (similarly as for the chance node).
            // Now even the player who didn't do anything might get an observation
            // (it was announced), just like in chance node case.
            if (hasNewOutcome()) {
                aoh[aoh.size() - 1].observation = lastObservation;
            }
            break;

        case TerminalNode:
            unreachable("parent cannot be terminal!");
        default:
            unreachable("unrecognized option");
    }

    // Prevent agent getting some information by appending "no information" :-)
    // Because we had some cases of appending NO_ACTION_OBSERVATION, if no observation was
    // actually made, it will be removed here.
    //
    // Except for the potential root NO_ACTION_OBSERVATION
    //   In this case no one will gain information by "no information",
    //   because it's prefix of every history.
    //   We add NO_ACTION_OBSERVATION to root to ensure that [aoh.size()-1] is always defined.
    if (aoh.size() > 1 && aoh[aoh.size() - 1] == NO_ACTION_OBSERVATION) aoh.pop_back();

    // Finally, player always gets observation that it's his move now.
    // This is done to ensure that infosets and aug infosets do not collide.
    // In the child nodes the action will be updated, and thus it will get a different AOs.
    if (type_ == PlayerNode && getPlayer() == player) {
        aoh.emplace_back(ActionObservationIds{NO_ACTION, observationPlayerMove(player)});
    }

    // Must always hold, after we made all of the above.
    //assert(algorithms::isAOCompatible(parent_->getAOids(player), aoh));
    //makes game run increeeeedibly slow!
    return aoh;
}

vector<ObservationId> FOG2EFGNode::getPubObsIds() const {
    if (!parent_) return vector<ObservationId>{};

    auto oh = parent_->getPubObsIds();

    const auto lastObservation = lastOutcome_->publicObservation->getId();

    // Always add new public observation if it occurs
    if (hasNewOutcome() && lastObservation != NO_OBSERVATION) {
        oh.push_back(lastObservation);
    }

    // Add that it's player's move, if it is not player's repeated move
    // If it is repeated, it means it might be secret.
    // If it's not secret, it should be revealed via new public observation.
    if (type_ == PlayerNode &&
        (parent_->type_ == ChanceNode
            || (parent_->type_ == PlayerNode && parent_->getPlayer() != getPlayer())
        ))
        oh.push_back(observationPlayerMove(getPlayer()));

    return oh;
}


const shared_ptr<FOG2EFGNode> FOG2EFGNode::getChildAt(EdgeId index) const {
    const auto action = availableActions().at(index);
    switch (type_) { // dispatch
        case ChanceNode:
            return performChanceAction(action);
        case PlayerNode:
            return performPlayerAction(action);
        case TerminalNode:
            unreachable("Cannot perform any actions in terminal node!");
        default:
            unreachable("unrecognized option!");
    }
}


shared_ptr<ActionSequence> FOG2EFGNode::getActionsSeqOfPlayer(Player player) const {
    vector<InfosetAction> actSeq = !parent_
                                   ? vector<InfosetAction>()
                                   : parent_->getActionsSeqOfPlayer(player)->sequence_;

    if (parent_ && parent_->currentPlayer_ == player) {
        actSeq.emplace_back(InfosetAction(parent_->getAOHInfSet(), incomingAction_));
    }
    return make_shared<ActionSequence>(actSeq);
}

double
FOG2EFGNode::getProbabilityOfActionSeq(Player player, const BehavioralStrategy &strat) const {
    if (!parent_) return 1.0;

    auto prob = parent_->getProbabilityOfActionSeq(player, strat);
    if (prob == 0.0 || parent_->type_ == ChanceNode) return prob;

    if (parent_->getPlayer() == player) {
        auto parentInfSet = parent_->getAOHInfSet();
        auto &actionsProbs = strat.at(parentInfSet);
        double actionProb = (actionsProbs.find(incomingAction_) != actionsProbs.end())
                            ? actionsProbs.at(incomingAction_) : 0.0;
        assert(actionProb <= 1.0);
        assert(actionProb >= 0.0);
        return prob * actionProb;
    } else {
        return prob;
    }
}
shared_ptr<Action> FOG2EFGNode::getActionByID(ActionId id) const {
    switch (type_) {
        case PlayerNode:
            return lastOutcome_->state->getActionByID(currentPlayer_, id);
        case ChanceNode:
            return createChanceActions()[id];
        case TerminalNode:
            unreachable("Not defined for terminal nodes!");
        default:
            unreachable("unrecognized option!");
    }
}

shared_ptr<EFGNode> createRootEFGNode(const OutcomeDistribution &rootOutcomes) {
    if (rootOutcomes.size() > 1) {
        return make_shared<FOG2EFGNode>(shared_ptr<FOG2EFGNode const>(), shared_ptr<Action>(),
                                        ChanceNode, shared_ptr<Outcome>(), rootOutcomes,
                                        vector<Player>(), vector<PlayerAction>(), 0);
    }

    // This is similar to EFGNode::createNodeForSpecificOutcome,
    // it's just that we do this for the root distribution
    const auto &[outcome, chanceProb] = rootOutcomes[0];
    assert(chanceProb == 1.0);
    const shared_ptr<State> &rootState = outcome.state;

    EFGNodeType childType = ChanceNode;
    if (rootState->isTerminal()) {
        unreachable("some weird game where no one plays and players get utility right away");
    } else if (!rootState->getPlayers().empty()) {
        childType = PlayerNode;
    }

    if (childType != ChanceNode) {
        // Enter new round without chance node.
        // Note that the state depth is set to 1 -- only the root chance node
        // gets to have state depth equal to 0
        return make_shared<FOG2EFGNode>(shared_ptr<FOG2EFGNode const>(), shared_ptr<Action>(),
                                        childType, make_shared<Outcome>(outcome),
                                        OutcomeDistribution(), rootState->getPlayers(),
                                        vector<PlayerAction>(), 1);
    }

    // We will return chance node, but we need to call NO_ACTION
    // for all players to get distribution for the chance player.
    const auto outcomeDistribution = rootState->performPartialActions(vector<PlayerAction>());
    return make_shared<FOG2EFGNode>(shared_ptr<FOG2EFGNode const>(), shared_ptr<Action>(),
                                    ChanceNode, make_shared<Outcome>(outcome), outcomeDistribution,
                                    vector<Player>(), vector<PlayerAction>(), 0);
}

}
