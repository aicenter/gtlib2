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

#include <base/algorithm.h>
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

struct decodedObservation;

decodedObservation decodeStrategoObservation(unsigned int obsid);
Rank getRank(CellState cell);

enum GameState {
    Setup, Playing, Finished
};

struct StrategoSettings {
    StrategoSettings() { sort(figures.begin(), figures.end()); }

    StrategoSettings(int height, int width, vector<Lake> _lakes,
                     const vector<pair<Rank, int>> &figureCounts) :
        boardHeight(height), boardWidth(width), lakes(move(_lakes)) {
        for (auto &[f, n] : figureCounts) {
            for (int i = 0; i < n; i++) figures.push_back(f);
        }
        sort(this->figures.begin(), this->figures.end());
    }

    StrategoSettings(int height, int width, vector<Lake> _lakes, vector<Rank> _figures) :
        boardHeight(height), boardWidth(width), lakes(move(_lakes)), figures(move(_figures)) {
        sort(this->figures.begin(), this->figures.end());
    }
    int boardHeight = 3;
    int boardWidth = 3;
    vector<Lake> lakes = {{1, 1, 1, 1}};

    // initial list of ranks available for player
    vector<Rank> figures = {'1', '2', '3'};
    vector<CellState> generateEmptyBoard() const;
    int getBoardSize() const { return boardHeight * boardWidth; };
};

struct StrategoConstraint: public Constraint {
    explicit StrategoConstraint(bool _moved) : moved(_moved) {};
    explicit StrategoConstraint(Rank rank) : revealedRank(rank) {};
 public:
    Rank revealedRank = EMPTY;
    bool moved = false;
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
    static unique_ptr<StrategoDomain> STRAT4x4();

    vector<Player> getPlayers() const { return {Player(0), Player(1)}; }

    const int boardWidth_;
    const int boardHeight_;
    const vector<Rank> startFigures_;
    const vector<CellState> emptyBoard_;
    bool updateConstraints(const shared_ptr<AOH> &currentInfoset, long &startIndex,
                           ConstraintsMap &revealedFigures) const override;
    void generateNodes(const shared_ptr<AOH> &currentInfoset,
                       const ConstraintsMap &revealedInfo, BudgetType budgetType,
                       int budget, const EFGNodeCallback &newNodeCallback) const override;
    void initializeEnumerativeConstraints(ConstraintsMap &) const override {};
    unsigned long inversePosition(unsigned long pos) const;

 private:
    void recursiveNodeGeneration(const shared_ptr <AOH> &currentInfoset,
                                 const shared_ptr <EFGNode> &node, const unsigned int depth,
                                 const vector <shared_ptr<StrategoConstraint>> &mask,
                                 const vector <Rank> &remaining,
                                 const BudgetType budgetType, int &counter,
                                 const EFGNodeCallback &newNodeCallback) const;
    void nodeGenerationTerminalPhase(const vector<ActionObservationIds> &aoids,
                                     const shared_ptr<EFGNode> &node,
                                     const EFGNodeCallback &newNodeCallback) const;
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
        : Action(id), figureRank_(rank), boardID_(board) {};
    inline string toString() const override;
    bool operator==(const Action &that) const override;
    inline HashType getHash() const override { return id_; };
    const Rank figureRank_;
    const int boardID_;
};

class StrategoMoveAction: public Action {
 public:
    StrategoMoveAction(ActionId id, int start, int end, int width)
        : Action(id), startPos_(start), endPos_(end), boardWidth_(width) {}
    inline string toString() const override;
    bool operator==(const Action &that) const override;
    inline HashType getHash() const override { return id_; };
    const unsigned int startPos_;
    const unsigned int endPos_;
    const unsigned int boardWidth_;
};

class StrategoMoveObservation: public Observation {
 public:
    StrategoMoveObservation(int startPos, int endPos, Rank startCell, Rank endCell);
    const unsigned int startPos_;
    const unsigned int endPos_;
    const Rank startCell_;
    const Rank endCell_;
};

class StrategoSetupObservation: public Observation {
 public:
    StrategoSetupObservation(Rank figureRank, int boardID, int playerID);
    const Rank figureRank_;
    const unsigned int boardID_;
    const unsigned int playerID_;
};

class StrategoState: public State {
 public:
    inline StrategoState(const Domain *domain, vector<CellState> boardState,
                         GameState(state), int player, int noAttackCounter) :
        State(domain,
              hashCombine(98612345434231,
                          boardState,
                          state == Setup,
                          state == Finished,
                          player,
                          noAttackCounter)),
        boardState_(move(boardState)), gameState_(state), currentPlayer_(player),
        domain_(dynamic_cast<const StrategoDomain *>(domain)), noAttackCounter_(noAttackCounter) {
    }

    void setFiguresToPlace(const vector<Rank> &figures) {
        remainingFiguresToPlace_[0] = figures;
        remainingFiguresToPlace_[1] = figures;
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

    const vector<CellState> boardState_;
    const GameState gameState_;
    const Player currentPlayer_;


 private:
    int boardIDToPlace_ = 0;
    array<vector<Rank>, 2> remainingFiguresToPlace_;
    const StrategoDomain *domain_;
    // noAttackCounter prevent games from looping. After a fixed number of moves without attacks game is stopped with a draw.
    const int noAttackCounter_;
};
}

#endif //GTLIB2_STRATEGO_H
