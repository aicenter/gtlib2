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


#ifndef GTLIB2_STRATEGO_H
#define GTLIB2_STRATEGO_H

#include "base/base.h"
#include "base/constrainingDomain.h"

namespace GTLib2::domains {

typedef uint8_t CellState;
typedef char Rank;

constexpr auto LAKE = CellState('L');
constexpr auto EMPTY = CellState(' ');

constexpr auto FLAG = Rank('F');
constexpr auto BOMB = Rank('B');
constexpr auto SAPPER = Rank('2');
constexpr auto SPY = Rank('0');
constexpr auto MARSHALL = Rank('9');

struct Lake {
    const int x, y, height, width;
};

vector<unsigned int> decodeStrategoObservation(unsigned int obsid);
Rank getRank(CellState cell);

enum GameState {
    Setup, Playing, Finished
};

struct StrategoSettings {
    int boardHeight = 3;
    int boardWidth = 3;
    vector<Lake> lakes = {{1, 1, 1, 1}};

    // initial list of ranks available for player
    vector<Rank> figures = {'1', '2', '3'};
    vector<CellState> generateEmptyBoard();
    int getBoardSize() const { return boardHeight * boardWidth; };
};

struct StrategoConstraint: public Constraint {
    explicit StrategoConstraint(bool moved) : moved(moved), revealedRank(EMPTY) {};
    explicit StrategoConstraint(Rank rank) : revealedRank(rank), moved(false) {};
 public:
    Rank revealedRank;
    bool moved;
};

/**
 * Basic rules:
 *
 * At each turn a figure can be moved to another cell or an attack action can be performed.
 * When one figure attacks another the figure with higher rank wins the fight (with some exceptions).
 *
 * If attacking figure won the fight, it will be moved to the cell of the attacked figure.
 * If attacking figure lost the fight, it will be removed from the board and the attacked figure wont be moved.
 * If both figures have the same rank, both will be removed.
 * If the attacked figure is a Bomb (B), the attacking figure loses this fight.
 *
 * Bombs can be only destroyed by figures called Sapper (2).
 * Marshal (9) can be only killed by Spy (0).
 * If the attacked figure is a Flag (F), owner of the attacking figure wins the game.
 * Flags and Bombs cannot move or perform attacks.
 * If a player has lost all his movable figures, he loses the game.
 * If both players have lost all their movable figure, it's a draw.
 *
 * emptyBoard_ contains only empty cells and lakes
 * startFigures_ are ranks available to every player they can place on the board during the setup action.
 */
class StrategoDomain: public Domain, public ConstrainingDomain {
 public:
    explicit StrategoDomain(StrategoSettings settings);
    string getInfo() const override;

    static unique_ptr<StrategoDomain> STRAT2x2();
    static unique_ptr<StrategoDomain> STRAT3x2();
    static unique_ptr<StrategoDomain> STRAT3x3();

    vector<Player> getPlayers() const {
        return {0, 1};
    }

    const int boardHeight_;
    const int boardWidth_;
    const vector<Rank> startFigures_;
    const vector<CellState> emptyBoard_;
    bool updateConstraints(const shared_ptr<AOH> &currentInfoset, long &startIndex,
                           ConstraintsMap &revealedFigures) const override;
    void generateNodes(const shared_ptr<AOH> &currentInfoset,
                       const ConstraintsMap &revealedInfo,
                       int max, const EFGNodeCallback &newNodeCallback) const override;
    void initializeEnumerativeConstraints(ConstraintsMap &revealedInfo) const override {};
 private:
    void recursiveNodeGeneration(const shared_ptr<AOH> &currentInfoset,
                                 const shared_ptr<EFGNode> &node, int depth,
                                 const vector<shared_ptr<StrategoConstraint>> &mask,
                                 const vector<Rank> &remaining, int &counter,
                                 const EFGNodeCallback &newNodeCallback) const;
    void simulateMoves(const vector<ActionObservationIds> &aoids,
                       shared_ptr<EFGNode> node, const EFGNodeCallback &newNodeCallback) const;
};

CellState createCell(Rank figure, Player player);
bool isFigureSlain(CellState attacker, CellState defender);

/**
 * Setup actions are permutations of startFigures_ of domain.
 * They are factorized over cell positions.
 *
 * Figures are placed from the left top cell to the right for player 0,
 * and right bottom cell to the right for player 1.
 */
class StrategoSetupAction: public Action {
 public:
    inline StrategoSetupAction(ActionId id, Rank rank, int board)
        : Action(id), figureRank(rank), boardID(board) {}
    inline string toString() const override;
    bool operator==(const Action &that) const override;
    inline HashType getHash() const override { return id_; };
    const Rank figureRank;
    const int boardID;
};

class StrategoMoveAction: public Action {
 public:
    inline StrategoMoveAction(ActionId id, int start, int end, int width)
        : Action(id), startPos(start), endPos(end), boardWidth_(width) {}
    inline string toString() const override;
    bool operator==(const Action &that) const override;
    inline HashType getHash() const override { return id_; };
    const int startPos;
    const int endPos;
    const int boardWidth_;
};

class StrategoMoveObservation: public Observation {
 public:
    inline StrategoMoveObservation() :
        Observation(), startPos_(0), endPos_(0), startCell_(0), endCell_(0) {}
    StrategoMoveObservation(int startPos, int endPos, Rank startCell, Rank endCell);
    const int startPos_;
    const int endPos_;
    const Rank startCell_;
    const Rank endCell_;
};

class StrategoSetupObservation: public Observation {
 public:
    inline StrategoSetupObservation() :
        Observation(), figureRank_(0), boardID_(0), playerID_(0) {}
    StrategoSetupObservation(Rank figureRank, int boardID, int playerID);
    const Rank figureRank_;
    const int boardID_;
    const int playerID_;
};

class StrategoState: public State {
 public:
    inline StrategoState(const Domain *domain, vector<CellState> boardState,
                         GameState(state), int player, int noAttackCounter) :
        State(domain, hashCombine(98612345434231, boardState, state == Setup, state == Finished, player, noAttackCounter)),
        boardState_(move(boardState)),
        gameState_(state),
        currentPlayer_(player),
        boardIDToPlace_(0),
        noAttackCounter_(noAttackCounter) {
    }

    unsigned long countAvailableActionsFor(Player player) const override;
    // for future use
    shared_ptr<Action> getActionByID(Player player, ActionId action) const override;
    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;
    OutcomeDistribution performMoveAction(const vector<shared_ptr<Action>> &actions) const;
    OutcomeDistribution performSetupAction(const vector<shared_ptr<Action>> &actions) const;
    OutcomeDistribution performActions(const vector<shared_ptr<Action>> &actions) const override;
    vector<Player> getPlayers() const override;
    bool isTerminal() const override;
    inline string toString() const override;
    bool operator==(const State &rhs) const override;

    // noAttackCounter prevent games from looping. After a fixed number of moves without attacks game is stopped with a draw.
    const int noAttackCounter_;
    const Player currentPlayer_;
    const GameState gameState_;
    const vector<CellState> boardState_;
    array<vector<Rank>, 2> remainingFiguresToPlace_;
    int boardIDToPlace_;
};
}

#endif //GTLIB2_STRATEGO_H
