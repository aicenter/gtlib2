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

    rootStatesDistribution[outcome] = 1.0;

//    vector<pair<Outcome, double>> distr;
//    distr.emplace_back(outcome,1.0);
//
//    rootStatesDistributionPtr = make_shared<OutcomeDistributionOld>(distr);

}

vector<int> MatchingPenniesDomain::getPlayers() const {
    return {0,1};
}

vector<shared_ptr<Action>> MatchingPenniesState::getAvailableActionsFor(int player) const {

    vector<shared_ptr<Action>> actions = vector<shared_ptr<Action>>();
    if (player == 0 && player1 == Nothing && player2 == Nothing) {
        actions.push_back(make_shared<MatchingPenniesAction>(Heads));
        actions.push_back(make_shared<MatchingPenniesAction>(Tails));
    } else if (player == 1 && player1 != Nothing && player2 == Nothing){
        actions.push_back(make_shared<MatchingPenniesAction>(Heads));
        actions.push_back(make_shared<MatchingPenniesAction>(Tails));
    }
    return actions;
}


MatchingPenniesState::MatchingPenniesState(Move p1, Move p2) : State() {
    player1 = p1;
    player2 = p2;
    if (player1 == Nothing && player2 == Nothing) {
        players.insert(0);
    }
    if (player2 == Nothing && player1 != Nothing) {
        players.insert(1);
    }

}



int MatchingPenniesState::getNumberOfPlayers() const {
    return (int) players.size();
}

OutcomeDistribution MatchingPenniesState::performActions(const unordered_map<int, shared_ptr<Action>> &actions) const {


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

    unordered_map<Outcome, double> distr;
    distr[outcome] = 1.0;

//    vector<pair<Outcome, double>> distr;
//    distr.emplace_back(outcome,1.0);

    return distr;
}



string MatchingPenniesState::toString(int player) const {

    std:: string desc = "Player 1: ";
    desc += player1 == Nothing ? "Nothing" : player1 == Heads ? "Heads" : "Tails";
    desc += " Player 2: ";
    desc += player2 == Nothing ? "Nothing" : player2 == Heads ? "Heads" : "Tails";

    return desc;
}

unordered_set<int> MatchingPenniesState::getPlayersSet() const {
    return players;
}

size_t MatchingPenniesState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, player1);
    boost::hash_combine(seed, player2);
    return seed;
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
    player1 = p1;
    player2 = p2;
    if (player1 == Nothing) {
        players.insert(0);
    }
    if (player2 == Nothing) {
        players.insert(1);
    }
}

vector<shared_ptr<Action>> SimultaneousMatchingPenniesState::getAvailableActionsFor(int player) const {
    vector<shared_ptr<Action>> actions = vector<shared_ptr<Action>>();
    if (player1 == Nothing && player2 == Nothing) {
        actions.push_back(make_shared<MatchingPenniesAction>(Heads));
        actions.push_back(make_shared<MatchingPenniesAction>(Tails));
    }
    return actions;
}




OutcomeDistribution SimultaneousMatchingPenniesState::performActions(const unordered_map<int, shared_ptr<Action>> &actions) const {

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

    unordered_map<Outcome, double> distr;
    //vector<pair<Outcome, double>> distr;
    //distr.emplace_back(outcome,1.0);

    distr[outcome] = 1.0;

    return distr;
}

int SimultaneousMatchingPenniesState::getNumberOfPlayers() const {
    return (int) players.size();
}


string SimultaneousMatchingPenniesState::toString(int player) const {
    std:: string desc = "Player 1: ";
    desc += player1 == Nothing ? "Nothing" : player1 == Heads ? "Heads" : "Tails";
    desc += " Player 2: ";
    desc += player2 == Nothing ? "Nothing" : player2 == Heads ? "Heads" : "Tails";

    return desc;
}

unordered_set<int> SimultaneousMatchingPenniesState::getPlayersSet() const {
    return players;
}

size_t SimultaneousMatchingPenniesState::getHash() const {
    size_t seed = 0;
    boost::hash_combine(seed, player1);
    boost::hash_combine(seed, player2);
    return seed;
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

    rootStatesDistribution[outcome] = 1.0;

//    vector<pair<Outcome, double>> distr;
//    distr.emplace_back(outcome,1.0);
//
//    rootStatesDistributionPtr = make_shared<OutcomeDistributionOld>(distr);

}

vector<int> SimultaneousMatchingPenniesDomain::getPlayers() const {
    return {0,1};
}


#pragma clang diagnostic pop


