//
// Created by Pavel Rytir on 08/01/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_MATCHING_PENNIES_H
#define PURSUIT_MATCHING_PENNIES_H


#include "../base/base.h"

namespace GTLib2 {

    class MatchingPenniesDomain : public Domain {
    public:
        MatchingPenniesDomain();

        string getInfo() const override { return "Matching pennies"; }

    };

    class SimultaneousMatchingPenniesDomain : public Domain {
    public:
        SimultaneousMatchingPenniesDomain();

        string getInfo() const override { return "Matching pennies"; }

    };

    enum Move {
        Heads, Tails, Nothing
    };
    enum OtherMove {
        OtherHeads, OtherTails, OtherNothing
    };

    class MatchingPenniesAction : public Action {
    public:
        explicit MatchingPenniesAction(Move moveParm);

        string toString() const override;

        Move move;
    };

    class MatchingPenniesObservation : public Observation {
    public:
        explicit MatchingPenniesObservation(OtherMove otherMoveParm);

        OtherMove otherMove;
    };

    class MatchingPenniesState : public State {
    public:
        MatchingPenniesState(Move p1, Move p2);

        vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;


        OutcomeDistribution performActions(const unordered_map<int, shared_ptr<Action>> &actions) const override;

        int getNumberOfPlayers() const override;
        unordered_set<int> getPlayersSet() const override ;
        size_t getHash() const override;


        Move player1;
        Move player2;
        unordered_set<int> players;


        // ToString returns state description
        string toString(int player) const override;

    };

    class SimultaneousMatchingPenniesState : public State {
    public:
        SimultaneousMatchingPenniesState(Move p1, Move p2);

        vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;


        OutcomeDistribution performActions(const unordered_map<int, shared_ptr<Action>> &actions) const override;

        int getNumberOfPlayers() const override;

        unordered_set<int> getPlayersSet() const override;

        size_t getHash() const override;


        Move player1;
        Move player2;
        unordered_set<int> players;

        // ToString returns state description
        string toString(int player) const override;

    };


}


#endif //PURSUIT_MATCHING_PENNIES_H

#pragma clang diagnostic pop