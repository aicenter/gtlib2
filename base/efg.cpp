//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"


namespace GTLib2 {


    EFGNode::EFGNode() {
        player_ = -1;
        parent = nullptr;
    }

    unordered_map<shared_ptr<EFGNode>, double> EFGNode::pavelPerformAction(shared_ptr<Action> action) const {

        unordered_map<int, shared_ptr<Action>> actionsToBePerformed(performedActionsInThisRound);
        actionsToBePerformed[*currentPlayer] = action;

        unordered_map<shared_ptr<EFGNode>, double> newNodes;

        if (remainingPlayersInTheRound.size() == 1) {
            //Last player in the round. So we proceed to the next state
            auto probDist = state->performActions(actionsToBePerformed);
            for (auto outcomeProb : probDist.distribution) {
                auto outcome = std::get<0>(outcomeProb);
                auto prob = std::get<1>(outcomeProb);
                auto newNode = make_shared<EFGNode>(outcome.GetState(), shared_from_this(),
                                                    outcome.observations, outcome.rewards,
                                                    actionsToBePerformed);
                newNodes[newNode] = prob;
            }
        } else {
            auto newNode = make_shared<EFGNode>(shared_from_this(), action, *currentPlayer, remainingPlayersInTheRound);
            newNodes[newNode] = 1.0;
        }
        return newNodes;
    }

    EFGNode::EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> lastNode,
                     const unordered_map<int, shared_ptr<Observation>> &observations,
                     const unordered_map<int, double> &rewards,
                     const unordered_map<int, shared_ptr<Action>> &lastRoundActions,
                     const unordered_map<int, shared_ptr<Observation>> &initialObservations) :
            EFGNode(newState, lastNode, observations,
                    rewards, lastRoundActions) {
        this->initialObservations = initialObservations;
    }

    EFGNode::EFGNode(shared_ptr<State> newState, shared_ptr<EFGNode const> lastNode,
                     const unordered_map<int, shared_ptr<Observation>> &observations,
                     const unordered_map<int, double> &rewards,
                     const unordered_map<int, shared_ptr<Action>> &lastRoundActions) {
        state = newState;
        this->observations = observations;
        this->lastRoundActions = lastRoundActions;
        this->rewards = rewards;
        parent = lastNode;
        remainingPlayersInTheRound = vector<int>(state->getPlayersSet().begin(), state->getPlayersSet().end());
        if (remainingPlayersInTheRound.size() > 0) {
            currentPlayer = *remainingPlayersInTheRound.begin();
            player_ = *remainingPlayersInTheRound.begin();
        } else {
            currentPlayer = nullopt;
            player_ = -1;
        }
        this->initialObservations = lastNode->initialObservations;

    }

    EFGNode::EFGNode(shared_ptr<EFGNode const> lastNode, const shared_ptr<Action> &lastAction, const int lastPlayer,
                     const vector<int> remainingPlayers) {
        state = lastNode->getState();
        this->observations = lastNode->observations;
        this->rewards = lastNode->rewards;
        parent = lastNode;
        performedActionsInThisRound[lastPlayer] = lastAction;
        std::copy_if(parent->remainingPlayersInTheRound.begin(),
                     parent->remainingPlayersInTheRound.end(),
                     std::back_inserter(remainingPlayersInTheRound),
                     [&lastPlayer](int i) { return i != lastPlayer; });

        if (remainingPlayersInTheRound.size() > 0) {
            currentPlayer = *remainingPlayersInTheRound.begin();
            player_ = *remainingPlayersInTheRound.begin();
        } else {
            currentPlayer = nullopt;
            player_ = -1;
        }
        this->initialObservations = lastNode->initialObservations;
    }

    vector<shared_ptr<Action>> EFGNode::availableActions() const {
        if (currentPlayer) {
            return state->getAvailableActionsFor(*currentPlayer);
        }
        return vector<shared_ptr<Action>>();
    }

    shared_ptr<AOH> EFGNode::pavelgetAOHInfSet() const {
        if (currentPlayer) {
            auto aoh = pavelGetAOH(*currentPlayer);
            int initObsId = initialObservations.at(*currentPlayer)->getId();
            return make_shared<AOH>(*currentPlayer, initObsId, aoh);
        } else {
            return nullptr;
        }
    }

    vector<std::tuple<int, int>> EFGNode::pavelGetAOH(int player) const {
        auto aoh = this->parent != nullptr ? this->parent->pavelGetAOH(player) : vector<std::tuple<int, int>>();


        if (currentPlayer && *currentPlayer == player) {
            auto action = lastRoundActions.find(player);
            auto observation = observations.find(player);


            if (action != lastRoundActions.end() && observation != observations.end()) {
                auto a = *action;
                auto o = *observation;
                aoh.emplace_back(action->second->getId(), observation->second->getId());
            }
        }
        return aoh;
    }

    optional<int> EFGNode::getCurrentPlayer() const {
        return currentPlayer;
    }

    ActionSequence EFGNode::getActionHistoryOfPlayer(int player) const {

        if (parent != nullptr) {
            return ActionSequence();
        }

        auto actSeq = parent->getActionHistoryOfPlayer(player);

        if (*parent->getCurrentPlayer() == player && lastRoundActions.find(player) != lastRoundActions.end()) {
            auto newAction = lastRoundActions.at(player);
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

    bool EFGNode::containedInInformationSet(const shared_ptr<AOH> &infSet) const {

        //TODO: more efficient implementation.
        auto mySet = this->pavelgetAOHInfSet();
        if (mySet == nullptr) {
            return false;
        }
        return *mySet == *infSet;
    }



    double EFGNode::getProbabilityOfSeqOfPlayer(int player, const BehavioralStrategy &strat) const {
        if (parent == nullptr) {
            return 1.0;
        }

        auto prob = parent->getProbabilityOfSeqOfPlayer(player, strat);

        if (*parent->getCurrentPlayer() == player && lastRoundActions.find(player) != lastRoundActions.end()) {
            auto lastAction = lastRoundActions.at(player);
            auto parentInfSet = parent->pavelgetAOHInfSet();
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

    unique_ptr<EFGNode> EFGNode::PerformAction(const shared_ptr<Action> &action2) {
        int player = player_ + 1;
        if (player >= rewards_.size())
            player -= rewards_.size();
        return MakeUnique<EFGNode>(player, state, rewards_, this);
    }

    vector<int> EFGNode::GetAOH(int player) const {
        if (parent == nullptr) {
            if (player == player_) {
                if (!last_.empty()) {
                    return {last_[0], last_[1]};
                }
            }
            return {};
        }
        vector<int> list = parent->GetAOH(player);
        if (player == player_) {
            if (!last_.empty()) {
                list.push_back(last_[0]);
                list.push_back(last_[1]);
            }
        }
        return list;
    }




}


#pragma clang diagnostic pop