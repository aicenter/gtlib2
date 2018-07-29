//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"

#include <utility>
#include <algorithm>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "TemplateArgumentsIssues"


namespace GTLib2 {




    EFGNodesDistribution EFGNode::performAction(shared_ptr<Action> action) const {

        unordered_map<int, shared_ptr<Action>> actionsToBePerformed(performedActionsInThisRound);
        actionsToBePerformed[*currentPlayer] = action;

        EFGNodesDistribution newNodes;

        if (remainingPlayersInTheRound.size() == 1) {
            for(auto &i :state->getDomain()->getPlayers()) {
              if (actionsToBePerformed.find(i) == actionsToBePerformed.end()){
                auto NoAction = make_shared<Action>(-1);
                actionsToBePerformed[i] = NoAction;
              }
            }
            //Last player in the round. So we proceed to the next state
            auto probDist = state->performActions(actionsToBePerformed);
            for(auto const& [outcome, prob] : probDist) {  // works in GCC 7.3
//            for (auto const& outcomeProb : probDist) {
//                auto outcome = outcomeProb.first;
//                auto prob = outcomeProb.second;
                auto newNode = make_shared<EFGNode>(outcome.state, shared_from_this(),
                                                    outcome.observations, outcome.rewards,
                                                    actionsToBePerformed, prob * natureProbability, action);
                newNodes.emplace_back(newNode,prob);
            }
        } else {
            auto newNode = make_shared<EFGNode>(shared_from_this(), actionsToBePerformed, *currentPlayer, action);
            newNodes.emplace_back(newNode,1.0);
        }
        return newNodes;
    }

    EFGNode::EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                     const unordered_map<int, shared_ptr<Observation>> &observations,
                     const unordered_map<int, double> &rewards,
                     const unordered_map<int, shared_ptr<Action>> &lastRoundActions,
                     double natureProbability, shared_ptr<Action> incomingAction,
                     const unordered_map<int, shared_ptr<Observation>> &initialObservations) :
            EFGNode(std::move(newState), std::move(parent), observations,
                    rewards, lastRoundActions, natureProbability, std::move(incomingAction)) {
        this->initialObservations = initialObservations;
    }

    EFGNode::EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                     const unordered_map<int, shared_ptr<Observation>> &observations,
                     const unordered_map<int, double> &rewards,
                     const unordered_map<int, shared_ptr<Action>> &lastRoundActions,
                     double natureProbability, shared_ptr<Action> incomingAction) {
        this->state = std::move(newState);
        this->observations = observations;
        this->previousRoundActions = lastRoundActions;
        this->rewards = rewards;
        this->natureProbability = natureProbability;


        auto statePlayers = state->getPlayers();
        remainingPlayersInTheRound = unordered_set<int>(statePlayers.begin(),statePlayers.end());
//        std::reverse(remainingPlayers.begin(), remainingPlayers.end());
        if (!remainingPlayersInTheRound.empty()) {
            currentPlayer = *remainingPlayersInTheRound.begin(); // TODO: zmenit poradi akorat
        } else {
            currentPlayer = nullopt;
        }
        this->parent = parent;
        this->incomingAction = std::move(incomingAction);
        if (parent != nullptr) {
            this->initialObservations = parent->initialObservations;
        }

    }

    EFGNode::EFGNode(shared_ptr<EFGNode const> parent, const unordered_map<int, shared_ptr<Action>> &performedActions,
                     const int lastPlayer, shared_ptr<Action> incomingAction) {
        this->state = parent->getState();
        this->observations = parent->observations;
        this->rewards = parent->rewards;
        this->natureProbability = parent->natureProbability;
        this->previousRoundActions = parent->previousRoundActions;
        this->initialObservations = parent->initialObservations;
        this->parent = parent;
        this->incomingAction = std::move(incomingAction);

        this->performedActionsInThisRound = performedActions;

        remainingPlayersInTheRound = parent->remainingPlayersInTheRound;
        remainingPlayersInTheRound.erase(lastPlayer);

        if (!remainingPlayersInTheRound.empty()) {
            currentPlayer = *remainingPlayersInTheRound.begin();
        } else {
            currentPlayer = nullopt;
        }
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
            int initObsId = initialObservations.at(*currentPlayer)->getId();
            return make_shared<AOH>(*currentPlayer, initObsId, aoh);
        } else {
            return nullptr;
        }
    }

    vector<std::pair<int, int>> EFGNode::getAOH(int player) const {
        auto aoh = this->parent != nullptr ? this->parent->getAOH(player) : vector<std::pair<int, int>>();
        if (remainingPlayersInTheRound.size() == 1) {
            auto action = previousRoundActions.find(player);
            auto observation = observations.find(player);
            if (action != previousRoundActions.end() && observation != observations.end()) {
                auto actionId = action->second->getId();
                auto observationId = observation->second->getId();
                aoh.emplace_back(actionId, observationId);
            }
        }
        return aoh;
    }

    optional<int> EFGNode::getCurrentPlayer() const {
        return currentPlayer;
    }

    ActionSequence EFGNode::getActionsSeqOfPlayer(int player) const {

        if (parent == nullptr) {
            return ActionSequence();
        }

        auto actSeq = parent->getActionsSeqOfPlayer(player);
        if(remainingPlayersInTheRound.size() == 1) {
          if (previousRoundActions.find(player) != previousRoundActions.end()) {
            auto newAction = previousRoundActions.at(player);
            actSeq.push_back(newAction);
          }
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

    shared_ptr<Action> EFGNode::getIncomingAction() const {
        return incomingAction;

    }

    double EFGNode::getProbabilityOfActionsSeqOfPlayer(int player, const BehavioralStrategy &strat) const {
        if (parent == nullptr) {
            return 1.0;
        }

        auto prob = parent->getProbabilityOfActionsSeqOfPlayer(player, strat);

        if (*parent->getCurrentPlayer() == player) { // TODO: i think it will not work with sequence games
            auto parentInfSet = parent->getAOHInfSet();
            auto actionsProbs = strat.at(parentInfSet);
            double actionProb = (actionsProbs.find(incomingAction) != actionsProbs.end()) ?
                                actionsProbs.at(incomingAction) : 0.0;
            return actionProb * prob;
        } else {
            return prob;
        }
    }

    size_t EFGNode::getHash() const {
        auto seed = state->getHash();
        boost::hash_combine(seed, performedActionsInThisRound.size());
        boost::hash_combine(seed, remainingPlayersInTheRound.size());
        return seed;
    }

    bool EFGNode::operator==(const EFGNode &rhs) const {
        if (this->performedActionsInThisRound.size() != rhs.performedActionsInThisRound.size()) {
            return false;
        }
        for (auto const& [player, action] : this->performedActionsInThisRound) { // works in GCC 7.3
//          for (auto const& it : this->performedActionsInThisRound) {
//            auto player = it.first;
//            auto action = it.second;
            if (rhs.performedActionsInThisRound.find(player) == rhs.performedActionsInThisRound.end()
                    || !(*rhs.performedActionsInThisRound.at(player) == *action)) {
                return false;
            }
        }
        bool remainingPlayersSame = this->remainingPlayersInTheRound == rhs.remainingPlayersInTheRound;
        bool innerStateSame = *(this->state) == *(rhs.state);
        return remainingPlayersSame && innerStateSame;
    }

    int EFGNode::getDistanceFromRoot() const {
        if (parent == nullptr) {
            return 0;
        } else {
            return 1 + parent->getDistanceFromRoot();
        }
    }

    int EFGNode::getLastObservationIdOfCurrentPlayer() const {
        return observations.at(*currentPlayer)->getId();
    }

    string EFGNode::toString() const {
            assert(("toString is not implemented", false));
            return std::string();
    }

}


#pragma clang diagnostic pop