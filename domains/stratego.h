#include <utility>

//
// Created by Nikita Milyukov on 2019-06-26.
//
#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

#ifndef GTLIB2_STRATEGO_H
#define GTLIB2_STRATEGO_H
#include "base/base.h"

namespace GTLib2::domains {

//enum ActionDirection {Left, Right, Up, Down};

struct Lake {
    unsigned char x, y, height, width;

};

struct StrategoSettings {
    int boardHeight = 3;
    int boardWidth = 3;
    vector<Lake> lakes = {{1, 1, 1, 1}};
    vector<unsigned char> figures = {'1', '2', '3'};
    vector<unsigned char> generateBoard();
    int getBoardSize() const {return boardHeight*boardWidth;};
};


class StrategoSetupAction: public Action {
public:
    inline StrategoSetupAction() : Action(), figuresSetup() {}
    inline StrategoSetupAction(ActionId id, vector<unsigned char> setup) : Action(id), figuresSetup(std::move(setup))  {}
    inline string toString() const override;
    bool operator==(const Action &that) const override;
    inline HashType getHash() const override { return id_; };
    const vector<unsigned char> figuresSetup;
};

class StrategoMoveAction: public Action {
public:
    inline StrategoMoveAction() : Action(), startPos(0), endPos(0)  {}
    inline StrategoMoveAction(ActionId id, int start, int end) : Action(id), startPos(start), endPos(end)  {}
    inline string toString() const override { return "moving from " + to_string(startPos) + " to " + to_string(endPos); };
    bool operator==(const Action &that) const override;
    inline HashType getHash() const override { return id_; };
    const int startPos;
    const int endPos;
};

class StrategoDomain: public Domain {
public:
    explicit StrategoDomain(StrategoSettings settings);
    string getInfo() const override;
    vector <Player> getPlayers() const {
        return {0, 1};
    }

    const int boardHeight_;
    const int boardWidth_;
    const vector<unsigned char> startFigures_;
    const vector<unsigned char> startBoard_;

private:
};

class StrategoObservation: public Observation {
public:
    inline StrategoObservation() :
            Observation(),
            startPos_(0),
            endPos_(0),
            startRank_(0),
            endRank_(0){}
    StrategoObservation(int startPos, int endPos, unsigned char startRank, unsigned char endRank);
    StrategoObservation(int id1, int id2);
    const int startPos_;
    const int endPos_;
    const unsigned char startRank_;
    const unsigned char endRank_;
};

class StrategoState: public State {
public:
    inline StrategoState(const Domain *domain, vector<unsigned char> boardState, bool setupState, bool finished, int player, int noAction) :
            State(domain, hashCombine(98612345434231, boardState, setupState, finished, player)),
            boardState_(move(boardState)),
            setupState_(setupState),
            isFinished_(finished),
            currentPlayer_(player),
            noActionCounter_(noAction){}

    unsigned long countAvailableActionsFor(Player player) const override;
    vector <shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performActions(const vector <shared_ptr<Action>> &actions) const override;
    vector <Player> getPlayers() const override;
    bool isTerminal() const override;
    inline string toString() const override;
    bool operator==(const State &rhs) const override;

    const int noActionCounter_;
    const Player currentPlayer_;
    const bool isFinished_;
    const bool setupState_;
    const vector<unsigned char> boardState_;
};
}

#endif //GTLIB2_STRATEGO_H
