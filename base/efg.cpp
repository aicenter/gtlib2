//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"

#include <utility>


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"


namespace GTLib2 {


    EFGNode::EFGNode() {
        player_ = -1;
        parent = nullptr;
    }

    unordered_map<shared_ptr<EFGNode>, double> EFGNode::performAction(shared_ptr<Action> action) const {

        unordered_map<int, shared_ptr<Action>> actionsToBePerformed(performedActionsInThisRound);
        actionsToBePerformed[*currentPlayer] = action;

        unordered_map<shared_ptr<EFGNode>, double> newNodes;

        if (remainingPlayersInTheRound.size() == 1) {
            //Last player in the round. So we proceed to the next state
            auto probDist = state->performActions(actionsToBePerformed);
            for (auto outcomeProb : probDist) {
                auto outcome = outcomeProb.first;
                auto prob = outcomeProb.second;
                auto newNode = make_shared<EFGNode>(outcome.state, shared_from_this(),
                                                    outcome.observations, outcome.rewards,
                                                    actionsToBePerformed, prob * natureProbability);
                newNodes[newNode] = prob;
            }
        } else {
            auto newNode = make_shared<EFGNode>(shared_from_this(), actionsToBePerformed, *currentPlayer);
            newNodes[newNode] = 1.0;
        }
        return newNodes;
    }

    EFGNode::EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                     const unordered_map<int, shared_ptr<Observation>> &observations,
                     const unordered_map<int, double> &rewards,
                     const unordered_map<int, shared_ptr<Action>> &lastRoundActions,
                     double natureProbability,
                     const unordered_map<int, shared_ptr<Observation>> &initialObservations) :
            EFGNode(std::move(newState), std::move(parent), observations,
                    rewards, lastRoundActions, natureProbability) {
        this->initialObservations = initialObservations;
    }

    EFGNode::EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> parent,
                     const unordered_map<int, shared_ptr<Observation>> &observations,
                     const unordered_map<int, double> &rewards,
                     const unordered_map<int, shared_ptr<Action>> &lastRoundActions,
                     double natureProbability) {
        this->state = std::move(newState);
        this->observations = observations;
        this->previousRoundActions = lastRoundActions;
        this->rewards = rewards;
        this->natureProbability = natureProbability;

        remainingPlayersInTheRound = vector<int>(state->getPlayersSet().begin(), state->getPlayersSet().end());
        if (!remainingPlayersInTheRound.empty()) {
            currentPlayer = *remainingPlayersInTheRound.begin();
            player_ = *remainingPlayersInTheRound.begin();
        } else {
            currentPlayer = nullopt;
            player_ = -1;
        }
        this->parent = parent;
        if (parent != nullptr) {
            this->initialObservations = parent->initialObservations;
        }

    }

    EFGNode::EFGNode(shared_ptr<EFGNode const> parent, const unordered_map<int, shared_ptr<Action>> &performedActions,
                     const int lastPlayer) {
        this->state = parent->getState();
        this->observations = parent->observations;
        this->rewards = parent->rewards;
        this->natureProbability = parent->natureProbability;
        this->previousRoundActions = parent->previousRoundActions;
        this->initialObservations = parent->initialObservations;
        this->parent = parent;

        this->performedActionsInThisRound = performedActions;

        //___performedActionsInThisRound[lastPlayer] = lastAction;
        std::copy_if(parent->remainingPlayersInTheRound.begin(),
                     parent->remainingPlayersInTheRound.end(),
                     std::back_inserter(remainingPlayersInTheRound),
                     [&lastPlayer](int i) { return i != lastPlayer; });

        if (!remainingPlayersInTheRound.empty()) {
            currentPlayer = *remainingPlayersInTheRound.begin();
            player_ = *remainingPlayersInTheRound.begin();
        } else {
            currentPlayer = nullopt;
            player_ = -1;
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

    vector<std::tuple<int, int>> EFGNode::getAOH(int player) const {
        auto aoh = this->parent != nullptr ? this->parent->getAOH(player) : vector<std::tuple<int, int>>();

        if (currentPlayer && *currentPlayer == player) {

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

        if (parent != nullptr) {
            return ActionSequence();
        }

        auto actSeq = parent->getActionsSeqOfPlayer(player);

        if (*parent->getCurrentPlayer() == player && previousRoundActions.find(player) != previousRoundActions.end()) {
            auto newAction = previousRoundActions.at(player);
            actSeq.push_back(newAction);
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

        //TODO: more efficient implementation.
        auto mySet = this->getAOHInfSet();
        if (mySet == nullptr) {
            return false;
        }
        return *mySet == *infSet;
    }



    double EFGNode::getProbabilityOfActionsSeqOfPlayer(int player, const BehavioralStrategy &strat) const {
        if (parent == nullptr) {
            return 1.0;
        }

        auto prob = parent->getProbabilityOfActionsSeqOfPlayer(player, strat);

        if (*parent->getCurrentPlayer() == player && previousRoundActions.find(player) != previousRoundActions.end()) {
            auto lastAction = previousRoundActions.at(player);
            auto parentInfSet = parent->getAOHInfSet();
            auto actionsProbs = strat.at(parentInfSet);
            double actionProb = (actionsProbs.find(lastAction) != actionsProbs.end()) ?
                                actionsProbs.at(lastAction) : 0.0;
            return actionProb*prob;
        } else {
            return prob;
        }
    }

    // Following deprecated ====================================

    EFGNode::EFGNode(int player, const shared_ptr<State> &state,
                     const vector<double> &rewards, EFGNode *node) :
            player_(player), state(state), rewards_(rewards), infset_(nullptr), parent(node) {}

    EFGNode::EFGNode(int player, const shared_ptr<State> &state,
                     const vector<double> &rewards, EFGNode *node,
                     vector<int> list) :
            player_(player), state(state), rewards_(rewards), infset_(nullptr),
            parent(node), last_(move(list)) {}


    vector<shared_ptr<Action>> EFGNode::GetAction() {
        vector<shared_ptr<Action>> list = state->getAvailableActionsFor(player_);
        return list;
    }

    unique_ptr<EFGNode> EFGNode::OldPerformAction(const shared_ptr<Action> &action2) {
        int player = player_ + 1;
        if (player >= rewards_.size())
            player -= rewards_.size();
        return MakeUnique<EFGNode>(player, state, rewards_, this);
    }

    vector<int> EFGNode::OldGetAOH(int player) const {
        if (parent == nullptr) {
            if (player == player_) {
                if (!last_.empty()) {
                    return {last_[0], last_[1]};
                }
            }
            return {};
        }
        vector<int> list = parent->OldGetAOH(player);
        if (player == player_) {
            if (!last_.empty()) {
                list.push_back(last_[0]);
                list.push_back(last_[1]);
            }
        }
        return list;
    }

    size_t EFGNode::getHash() const {
        auto seed = state->getHash();
        boost::hash_combine(seed, remainingPlayersInTheRound);
        return seed;
    }

    bool EFGNode::operator==(const EFGNode &rhs) const {
        return this->state == rhs.state && remainingPlayersInTheRound == rhs.remainingPlayersInTheRound;
    }


}


#pragma clang diagnostic pop