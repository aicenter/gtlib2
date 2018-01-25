//
// Created by Pavel Rytir on 08/01/2018.
//

#include "matching_pennies.h"
#include <assert.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"


using namespace GTLib2;

MatchingPenniesDomain::MatchingPenniesDomain() : Domain(std::numeric_limits<int>::max(),2) {
    shared_ptr<MatchingPenniesState>  rootState = make_shared<MatchingPenniesState>(Nothing,Nothing);

    shared_ptr<MatchingPenniesObservation> newObservationP1 = make_shared<MatchingPenniesObservation>(OtherNothing);
    shared_ptr<MatchingPenniesObservation> newObservationP2 = make_shared<MatchingPenniesObservation>(OtherNothing);

    unordered_map<int,shared_ptr<Observation>> observations = unordered_map<int,shared_ptr<Observation>>();

    observations[0] = newObservationP1;
    observations[1] = newObservationP2;

    unordered_map<int,double> rewards;

    rewards[0] = 0;
    rewards[1] = 0;

    Outcome outcome(rootState,observations,rewards);

    vector<pair<Outcome, double>> distr;
    distr.emplace_back(outcome,1.0);

    rootStatesDistributionPtr = make_shared<ProbDistribution>(distr);

}

vector<shared_ptr<Action>> MatchingPenniesState::getAvailableActionsFor(int player) const {

    vector<shared_ptr<Action>> actions = vector<shared_ptr<Action>>();
    GetActions(actions, player);
    return actions;
}

void MatchingPenniesState::GetActions(vector<shared_ptr<Action>> &list, int player) const {
    if (player == 0 && player1 == Nothing && player2 == Nothing) {
        list.push_back(make_shared<MatchingPenniesAction>(Heads));
        list.push_back(make_shared<MatchingPenniesAction>(Tails));
    } else if (player == 1 && player1 != Nothing && player2 == Nothing){
        list.push_back(make_shared<MatchingPenniesAction>(Heads));
        list.push_back(make_shared<MatchingPenniesAction>(Tails));
    }
}

MatchingPenniesState::MatchingPenniesState(Move p1, Move p2) : State() {
    players = vector<bool>(2);
    player1 = p1;
    player2 = p2;
    players[0] = player1 == Nothing && player2 == Nothing;
    players[1] = player2 == Nothing && player1 != Nothing;
}

const vector<bool> &MatchingPenniesState::GetPlayers() const {
    return players;
}

int MatchingPenniesState::getNumberOfPlayers() const {
    const int numPlayers = players[0] ? 1 : 0 + players[1] ? 1 : 0;
    return numPlayers;
}

ProbDistribution MatchingPenniesState::performActions(const unordered_map<int, shared_ptr<Action>> &actions) const {


    auto a1 = actions.find(0) != actions.end() ? actions.at(0) : nullptr;
    auto a2 = actions.find(1) != actions.end() ? actions.at(1) : nullptr;

    shared_ptr<MatchingPenniesAction> p1Action = std::dynamic_pointer_cast<MatchingPenniesAction>(a1);
    shared_ptr<MatchingPenniesAction> p2Action = std::dynamic_pointer_cast<MatchingPenniesAction>(a2);


    assert(p1Action == nullptr || p2Action == nullptr); // Only one action can be performed
    assert(player1 == Nothing || p2Action != nullptr ); //  player1 played -> player2 has to play.


    shared_ptr<MatchingPenniesState>  newState = make_shared<MatchingPenniesState>(
            p1Action == nullptr? player1 : p1Action->move, p2Action == nullptr ? Nothing : p2Action->move);

    const bool finalState = newState->player1 != Nothing && newState->player2 != Nothing;

    OtherMove p1obs = finalState ? ( newState->player2 == Heads ? OtherHeads : OtherTails ) : OtherNothing;
    OtherMove p2obs = finalState ? ( newState->player1 == Heads ? OtherHeads : OtherTails ) : OtherNothing;

    shared_ptr<MatchingPenniesObservation> newObservationP1 = make_shared<MatchingPenniesObservation>(p1obs);
    shared_ptr<MatchingPenniesObservation> newObservationP2 = make_shared<MatchingPenniesObservation>(p2obs);

    unordered_map<int,shared_ptr<Observation>> observations = unordered_map<int,shared_ptr<Observation>>();

    observations[0] = newObservationP1;
    observations[1] = newObservationP2;

    unordered_map<int,double> rewards = unordered_map<int,double>();
    if (newState->player1 == Nothing || newState->player2 == Nothing) {
        rewards[0] = 0;
        rewards[1] = 0;
    } else {
        if (newState->player1 == newState->player2) {
            rewards[0] = +1;
            rewards[1] = -1;
        } else {
            rewards[0] = -1;
            rewards[1] = +1;
        }
    }

    Outcome outcome(newState,observations,rewards);

    vector<pair<Outcome, double>> distr;
    distr.emplace_back(outcome,1.0);

    return ProbDistribution(distr);
}

ProbDistribution MatchingPenniesState::PerformAction(const vector<shared_ptr<Action>> &actionsParm) {


    // TODO: make an assert that only one action is performed
    // TODO: remove body of this function and call performActions(unordered_map<int, shared_ptr<Action>> &actions)


    unordered_map<int, shared_ptr<Action>> actions;

    actions[0] = actionsParm[0];
    actions[1] = actionsParm[1];

    return performActions(actions);

}

string MatchingPenniesState::toString(int player) {

    std:: string desc = "Player 1: ";
    desc += player1 == Nothing ? "Nothing" : player1 == Heads ? "Heads" : "Tails";
    desc += " Player 2: ";
    desc += player2 == Nothing ? "Nothing" : player2 == Heads ? "Heads" : "Tails";

    return desc;
}




MatchingPenniesObservation::MatchingPenniesObservation(OtherMove otherMoveParm) :
        Observation(static_cast<int>(otherMoveParm)) {

    otherMove = otherMoveParm;
}

MatchingPenniesAction::MatchingPenniesAction(Move moveParm) :
    Action(static_cast<int>(moveParm)){
    move = moveParm;
}

string MatchingPenniesAction::toString() const{
    return move == Heads ? "Heads" : move == Tails ? "Tails" : "Nothing";
}




SimultaneousMatchingPenniesState::SimultaneousMatchingPenniesState(Move p1, Move p2) : State() {
    assert((p1 != Nothing && p2 != Nothing) || (p1 == Nothing && p2 == Nothing));
    players = vector<bool>(2);
    player1 = p1;
    player2 = p2;
    players[0] = player1 == Nothing ;
    players[1] = player2 == Nothing;
}

vector<shared_ptr<Action>> SimultaneousMatchingPenniesState::getAvailableActionsFor(int player) const {
    vector<shared_ptr<Action>> actions = vector<shared_ptr<Action>>();
    GetActions(actions, player);
    return actions;
}

void SimultaneousMatchingPenniesState::GetActions(vector<shared_ptr<Action>> &list, int player) const {
    if (player1 == Nothing && player2 == Nothing) {
        list.push_back(make_shared<MatchingPenniesAction>(Heads));
        list.push_back(make_shared<MatchingPenniesAction>(Tails));
    }

}

ProbDistribution SimultaneousMatchingPenniesState::PerformAction(const vector<shared_ptr<Action>> &actionsParm) {
    unordered_map<int, shared_ptr<Action>> actions;

    actions[0] = actionsParm[0];
    actions[1] = actionsParm[1];

    return performActions(actions);
}

ProbDistribution SimultaneousMatchingPenniesState::performActions(const unordered_map<int, shared_ptr<Action>> &actions) const {

    auto a1 = actions.find(0) != actions.end() ? actions.at(0) : nullptr;
    auto a2 = actions.find(1) != actions.end() ? actions.at(1) : nullptr;

    shared_ptr<MatchingPenniesAction> p1Action = std::dynamic_pointer_cast<MatchingPenniesAction>(a1);
    shared_ptr<MatchingPenniesAction> p2Action = std::dynamic_pointer_cast<MatchingPenniesAction>(a2);


    assert(p1Action != nullptr || p2Action != nullptr); // Both action must be performed
    //assert(player1 == Nothing || p2Action != nullptr ); //  player1 played -> player2 has to play.


    shared_ptr<MatchingPenniesState>  newState = make_shared<MatchingPenniesState>(
            p1Action == nullptr? player1 : p1Action->move, p2Action == nullptr ? Nothing : p2Action->move);

    const bool finalState = newState->player1 != Nothing && newState->player2 != Nothing;

    OtherMove p1obs = finalState ? ( newState->player2 == Heads ? OtherHeads : OtherTails ) : OtherNothing;
    OtherMove p2obs = finalState ? ( newState->player1 == Heads ? OtherHeads : OtherTails ) : OtherNothing;

    shared_ptr<MatchingPenniesObservation> newObservationP1 = make_shared<MatchingPenniesObservation>(p1obs);
    shared_ptr<MatchingPenniesObservation> newObservationP2 = make_shared<MatchingPenniesObservation>(p2obs);

    unordered_map<int,shared_ptr<Observation>> observations = unordered_map<int,shared_ptr<Observation>>();

    observations[0] = newObservationP1;
    observations[1] = newObservationP2;

    unordered_map<int,double> rewards = unordered_map<int,double>();
    if (newState->player1 == Nothing || newState->player2 == Nothing) {
        rewards[0] = 0;
        rewards[1] = 0;
    } else {
        if (newState->player1 == newState->player2) {
            rewards[0] = +1;
            rewards[1] = -1;
        } else {
            rewards[0] = -1;
            rewards[1] = +1;
        }
    }

    Outcome outcome(newState,observations,rewards);

    vector<pair<Outcome, double>> distr;
    distr.emplace_back(outcome,1.0);

    return ProbDistribution(distr);
}

int SimultaneousMatchingPenniesState::getNumberOfPlayers() const {
    const int numPlayers = players[0] ? 1 : 0 + players[1] ? 1 : 0;
    return numPlayers;
}

const vector<bool> &SimultaneousMatchingPenniesState::GetPlayers() const {
    return players;
}

string SimultaneousMatchingPenniesState::toString(int player) {
    std:: string desc = "Player 1: ";
    desc += player1 == Nothing ? "Nothing" : player1 == Heads ? "Heads" : "Tails";
    desc += " Player 2: ";
    desc += player2 == Nothing ? "Nothing" : player2 == Heads ? "Heads" : "Tails";

    return desc;
}

SimultaneousMatchingPenniesDomain::SimultaneousMatchingPenniesDomain() : Domain(std::numeric_limits<int>::max(),2) {
    shared_ptr<SimultaneousMatchingPenniesState>  rootState = make_shared<SimultaneousMatchingPenniesState>(Nothing,Nothing);

    shared_ptr<MatchingPenniesObservation> newObservationP1 = make_shared<MatchingPenniesObservation>(OtherNothing);
    shared_ptr<MatchingPenniesObservation> newObservationP2 = make_shared<MatchingPenniesObservation>(OtherNothing);

    unordered_map<int,shared_ptr<Observation>> observations = unordered_map<int,shared_ptr<Observation>>();

    observations[0] = newObservationP1;
    observations[1] = newObservationP2;

    unordered_map<int,double> rewards;

    rewards[0] = 0;
    rewards[1] = 0;

    Outcome outcome(rootState,observations,rewards);

    vector<pair<Outcome, double>> distr;
    distr.emplace_back(outcome,1.0);

    rootStatesDistributionPtr = make_shared<ProbDistribution>(distr);

}


#pragma clang diagnostic pop


