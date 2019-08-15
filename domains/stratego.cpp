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


#include "domains/stratego.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2::domains {

// startpos/endpos < 512
unsigned int encodeObservation(int startPos, int endPos, CellState startCell, CellState endCell) {
    // 32 bits total
    // max sizes of stratego boards are 10x10 = 100, so pos < 8bits = 256
    return (startPos << 24)
        | (endPos << 16)
        | (startCell << 8) // 8 bits
        | endCell; // 8 bits
}

int maxMovesWithoutAttack(int h, int w) {
    return 2 * h + 2 * w - 4;
}

bool isPlayers(CellState cell, Player player) {
    if (cell == LAKE) return false;
    if (cell == EMPTY) return false;
    return (((player == Player(0)) && (cell < 128))
        || ((player == Player(1)) && (cell >= 128)))
        && (cell != LAKE);
}

CellState createCell(Rank figure, Player player) {
    return player == Player(0) ? figure : figure + 128;
}

bool isSamePlayer(CellState cell1, CellState cell2) {
    if (cell1 == EMPTY || cell2 == EMPTY) return false;
    return (isPlayers(cell1, Player(0)) && isPlayers(cell2, Player(0)))
        || (isPlayers(cell1, Player(1)) && isPlayers(cell2, Player(1)));
}

Rank getRank(CellState cell) {
    return cell < 128 ? cell : cell - 128;
}

bool isFigureSlain(CellState attacker, CellState defender) {
    assert(attacker != EMPTY);
    if (defender == EMPTY) return true;

    char rank1 = getRank(attacker);
    char rank2 = getRank(defender);
    if (rank2 == BOMB) return rank1 == SAPPER;
    if (rank1 == SPY && rank2 == MARSHALL) return true;

    return rank1 > rank2;
}

bool StrategoSetupAction::operator==(const Action &that) const {
    if (typeid(that) == typeid(*this)) {
        const auto rhsAction = dynamic_cast<const StrategoSetupAction *>(&that);
        return (figuresSetup == rhsAction->figuresSetup);
    }
    return false;
}

string StrategoSetupAction::toString() const {
    string out;
    for (auto &i : figuresSetup) {
        out += getRank(i);
        out += ' ';
    }
    return "figures setup: " + out;
}

bool StrategoMoveAction::operator==(const Action &that) const {
    if (typeid(that) == typeid(*this)) {
        const auto rhsAction = dynamic_cast<const StrategoMoveAction *>(&that);
        return (startPos == rhsAction->startPos) && (endPos == rhsAction->endPos);
    }
    return false;
}

string StrategoMoveAction::toString() const {

    return "move: (" + to_string(startPos % boardWidth_) + "," + to_string(startPos / boardWidth_)
        + ") -> (" + to_string(endPos % boardWidth_) + "," + to_string(endPos / boardWidth_) + ")";
}

vector<CellState> StrategoSettings::generateBoard() {
    vector<CellState> board(getBoardSize());
    fill(board.begin(), board.end(), ' ');
    for (auto &lake : lakes) {
        for (int i = 0; i < lake.height; i++) {
            for (int j = 0; j < lake.width; j++) {
                board[(lake.y + i) * boardWidth + lake.x + j] = LAKE;
            }
        }
    }
    return board;
}

StrategoObservation::StrategoObservation(const int startPos, const int endPos,
                                         const Rank startCell, const Rank endCell)
    : Observation(),
      startPos_(startPos), endPos_(endPos),
      startCell_(startCell), endCell_(endCell) {

    assert(startPos + endPos > 0);
    id_ = encodeObservation(startPos_, endPos_, startCell_, endCell_);
}


StrategoDomain::StrategoDomain(StrategoSettings settings) :
    Domain(maxMovesWithoutAttack(settings.boardHeight, settings.boardWidth)
               * settings.figures.size() * 2, 2, true,
           make_shared<Action>(), make_shared<StrategoObservation>()),
    emptyBoard_(settings.generateBoard()),
    startFigures_(settings.figures),
    boardWidth_(settings.boardWidth),
    boardHeight_(settings.boardHeight) {
    assert(boardHeight_ * boardWidth_ > 1);
    const auto newState = make_shared<StrategoState>(this, emptyBoard_, true, false, 0, 0);
    Outcome outcome(newState, {noObservation_, noObservation_}, noObservation_, {0.0, 0.0});
    maxUtility_ = 1.0;
    rootStatesDistribution_.emplace_back(OutcomeEntry(outcome));
}

string StrategoDomain::getInfo() const {
    return "Stratego game with " + to_string(emptyBoard_.size()) + " cells";
}

int fact(int n) {
    if (n == 0) return 1;
    if (n == 1) return 1;
    return n * fact(n - 1);
}

bool canMoveUp(int i, const vector<CellState> &board, int height, int width) {
    return (height > 1)
        && ((i + 1) > width)
        && !isSamePlayer(board[i], board[i - width])
        && (board[i - width] != LAKE);
}

bool canMoveDown(int i, const vector<CellState> &board, int height, int width) {
    return ((height > 1)
        && (board.size() - (i + 1) >= width)
        && !isSamePlayer(board[i], board[i + width])
        && (board[i + width] != LAKE));
}

bool canMoveLeft(int i, const vector<CellState> &board, int width) {
    return ((width > 1)
        && ((i + 1) % width != 1)
        && !isSamePlayer(board[i], board[i - 1])
        && (board[i - 1] != LAKE));
}

bool canMoveRight(int i, const vector<CellState> &board, int width) {
    return ((width > 1)
        && ((i + 1) % width != 0)
        && !isSamePlayer(board[i], board[i + 1])
        && (board[i + 1] != LAKE));
}

unsigned long StrategoState::countAvailableActionsFor(Player player) const {
    const auto stratDomain = dynamic_cast<const StrategoDomain *>(domain_);
    int height = stratDomain->boardHeight_;
    int width = stratDomain->boardWidth_;
    if (isSetupState_) {
        return fact(stratDomain->startFigures_.size());
    }

    int count = 0;
    for (int i = 0; i < boardState_.size(); i++) {
        if (!isPlayers(boardState_[i], player)
            || getRank(boardState_[i]) == BOMB
            || getRank(boardState_[i]) == FLAG)
            continue;

        if (canMoveUp(i, boardState_, height, width)) count++;
        if (canMoveRight(i, boardState_, width)) count++;
        if (canMoveLeft(i, boardState_, width)) count++;
        if (canMoveDown(i, boardState_, height, width)) count++;
    }
    return count;
}

shared_ptr<Action> StrategoState::getActionByID(const Player player, ActionId action) const {
    const auto stratDomain = dynamic_cast<const StrategoDomain *>(domain_);

    int id = 0;
    int height = stratDomain->boardHeight_;
    int width = stratDomain->boardWidth_;
    if (isSetupState_) {
        vector<Rank> comb = dynamic_cast<const StrategoDomain *>(getDomain())->startFigures_;
        do {
            if (id == action) return make_shared<StrategoSetupAction>(id++, comb);
        } while (next_permutation(comb.begin(), comb.end()));

        return make_shared<Action>();
    }

    for (int i = 0; i < boardState_.size(); i++) {
        if (!isPlayers(boardState_[i], player)
            || getRank(boardState_[i]) == BOMB
            || getRank(boardState_[i]) == FLAG)
            continue;

        if (canMoveUp(i, boardState_, height, width)) {
            if (id == action) return make_shared<StrategoMoveAction>(id++, i, i - width, width);
            id++;
        }
        if (canMoveRight(i, boardState_, width)) {
            if (id == action) return make_shared<StrategoMoveAction>(id++, i, i + 1, width);
            id++;
        }
        if (canMoveLeft(i, boardState_, width)) {
            if (id == action) return make_shared<StrategoMoveAction>(id++, i, i - 1, width);
            id++;
        }
        if (canMoveDown(i, boardState_, height, width)) {
            if (id == action) return make_shared<StrategoMoveAction>(id++, i, i + width, width);
            id++;
        }
    }

    unreachable("action id out of range!");
}

vector<shared_ptr<Action>> StrategoState::getAvailableActionsFor(const Player player) const {
    vector<shared_ptr<Action>> actions;
    const auto stratDomain = dynamic_cast<const StrategoDomain *>(domain_);
    int id = 0;
    int height = stratDomain->boardHeight_;
    int width = stratDomain->boardWidth_;

    if (isSetupState_) {
        vector<Rank> comb = dynamic_cast<const StrategoDomain *>(getDomain())->startFigures_;
        do {
            actions.push_back(make_shared<StrategoSetupAction>(id++, comb));
        } while (next_permutation(comb.begin(), comb.end()));
        return actions;
    }

    for (int i = 0; i < boardState_.size(); i++) {
        if (!isPlayers(boardState_[i], player)
            || getRank(boardState_[i]) == BOMB
            || getRank(boardState_[i]) == FLAG)
            continue;

        if (canMoveUp(i, boardState_, height, width))
            actions.push_back(make_shared<StrategoMoveAction>(id++, i, i - width, width));
        if (canMoveRight(i, boardState_, width))
            actions.push_back(make_shared<StrategoMoveAction>(id++, i, i + 1, width));
        if (canMoveLeft(i, boardState_, width))
            actions.push_back(make_shared<StrategoMoveAction>(id++, i, i - 1, width));
        if (canMoveDown(i, boardState_, height, width))
            actions.push_back(make_shared<StrategoMoveAction>(id++, i, i + width, width));
    }

    return actions;
}

vector<Player> StrategoState::getPlayers() const {
    if (isSetupState_) return {0, 1};
    else if (isFinished_) return {};
    return {currentPlayer_};
}

string StrategoState::toString() const {
    int w = dynamic_cast<const StrategoDomain *>(getDomain())->boardWidth_,
        h = dynamic_cast<const StrategoDomain *>(getDomain())->boardHeight_;

    string ret = "Pl: " + to_string(currentPlayer_) + "\n" +
        "No attacks: " + to_string(noAttackCounter_) + "\n";
    for (int i = 0; i < h; i++) {
        ret += "\n";
        for (int j = 0; j < w; j++) {
            CellState fig = boardState_[w * i + j];
            if (fig == EMPTY) ret += "__";
            else if (fig == LAKE) ret += "LL";
            else {
                ret += isPlayers(fig, 0) ? '0' : '1';
                ret += getRank(fig);
            }
            ret += ' ';
        }
    }
    return ret;
}

OutcomeDistribution
StrategoState::performSetupAction(const vector<shared_ptr<Action>> &actions) const {
    const auto stratDomain = dynamic_cast<const StrategoDomain *>(domain_);

    StrategoSetupAction
        actionpl0 = dynamic_cast<StrategoSetupAction &>(*actions[0]); // player 0 setup
    StrategoSetupAction
        actionpl1 = dynamic_cast<StrategoSetupAction &>(*actions[1]); // player 1 setup
    vector<CellState> board = stratDomain->emptyBoard_;
    for (int i = 0; i < actionpl0.figuresSetup.size(); i++) {
        board[i] = createCell(actionpl0.figuresSetup[i], 0);
        board[board.size() - 1 - i] = createCell(actionpl1.figuresSetup[i], 1);
    }

    const auto newState = make_shared<StrategoState>(stratDomain, board, false, false, 0, 0);
    const auto &noObs = stratDomain->getNoObservation();
    const auto newOutcome = Outcome(newState, {noObs, noObs}, noObs, {0, 0});

    return OutcomeDistribution{OutcomeEntry(newOutcome)};
}

vector<CellState> updateBoard(const vector<CellState> &oldBoard, int start, int end) {
    vector<CellState> board = oldBoard;

    const CellState &startCell = oldBoard.at(start);
    const CellState &endCell = oldBoard.at(end);

    if (getRank(endCell) == FLAG) {
        // Flag captured!
        board[start] = EMPTY;
        board[end] = startCell;
    } else if (getRank(startCell) == getRank(endCell)) {
        // Both units killed
        board[start] = EMPTY;
        board[end] = EMPTY;
    } else if (isFigureSlain(startCell, endCell)) {
        // Attack move! attacker wins
        board[start] = EMPTY;
        board[end] = startCell;
    } else {
        // Attack move! defender wins
        board[start] = EMPTY;
    }

    return board;
}

pair<bool, bool> checkOnlyOneMovablePieceRemains(const vector<CellState> &newBoard) {
    CellState pl0fig = EMPTY, pl1fig = EMPTY;
    int pl0MovableCounter = 0, pl1MovableCounter = 0;
    for (CellState f : newBoard) {
        if (f == EMPTY || f == LAKE || getRank(f) == BOMB || getRank(f) == FLAG) continue;

        if (isPlayers(f, 0)) {
            pl0MovableCounter++;
            pl0fig = pl0fig > getRank(f) ? pl0fig : getRank(f);
        }
        if (isPlayers(f, 1)) {
            pl1MovableCounter++;
            pl1fig = pl1fig > getRank(f) ? pl1fig : getRank(f);
        }
    }

    if (pl0MovableCounter == 0 || pl1MovableCounter == 0) {
        return make_pair(pl1MovableCounter == 0, pl0MovableCounter == 0);
    }
    if (pl0MovableCounter == 1 && pl1MovableCounter == 1) {
        return make_pair(pl0fig >= pl1fig, pl1fig >= pl0fig);
    }
    return make_pair(false, false);
}

OutcomeDistribution
StrategoState::performMoveAction(const vector<shared_ptr<Action>> &actions) const {
    const auto stratDomain = dynamic_cast<const StrategoDomain *>(domain_);
    StrategoMoveAction action = dynamic_cast<StrategoMoveAction &>(*actions[currentPlayer_]);

    const CellState &startCell = boardState_.at(action.startPos);
    const CellState &endCell = boardState_.at(action.endPos);
    assert(startCell != EMPTY && startCell != LAKE);

    vector<CellState> newBoard = updateBoard(boardState_, action.startPos, action.endPos);
    bool pl0won = false, pl1won = false;

    if (getRank(endCell) == FLAG) { // Flag captured!
        if (currentPlayer_ == 0) pl0won = true;
        else pl1won = true;
    } else {
        tie(pl0won, pl1won) = checkOnlyOneMovablePieceRemains(newBoard);
    }

    const vector<double> newRewards = {(pl0won ? 1.0 : 0.0) + (pl1won ? (-1.0) : 0.0),
                                       (pl1won ? 1.0 : 0.0) + (pl0won ? (-1.0) : 0.0)};

    shared_ptr<StrategoObservation> obs = make_shared<StrategoObservation>(
        action.startPos, action.endPos,
        endCell == EMPTY ? EMPTY : startCell, // do not reveal rank if moving to empty cell
        endCell); // reveal rank if it wasn't empty

    if (endCell == EMPTY && (noAttackCounter_ ==
        maxMovesWithoutAttack(stratDomain->boardWidth_, stratDomain->boardHeight_))) {

        const auto newState = make_shared<StrategoState>(stratDomain, newBoard, false, true,
                                                         opponent(currentPlayer_), 0);
        const auto newOutcome = Outcome(newState, {obs, obs}, obs, newRewards);
        return OutcomeDistribution{OutcomeEntry(newOutcome)};
    } else {
        const auto newState = make_shared<StrategoState>(
            stratDomain, newBoard, false, pl0won || pl1won, opponent(currentPlayer_),
            (endCell == EMPTY) ? noAttackCounter_ + 1 : 0);
        const auto newOutcome = Outcome(newState, {obs, obs}, obs, newRewards);
        return OutcomeDistribution{OutcomeEntry(newOutcome)};
    }
}

OutcomeDistribution StrategoState::performActions(const vector<shared_ptr<Action>> &actions) const {
    if (isSetupState_) return performSetupAction(actions);
    else return performMoveAction(actions);
}

bool StrategoState::isTerminal() const { return isFinished_; }

bool StrategoState::operator==(const State &rhs) const {
    auto state = dynamic_cast<const StrategoState &>(rhs);

    return hash_ == state.hash_
        && currentPlayer_ == state.currentPlayer_
        && isSetupState_ == state.isSetupState_
        && isFinished_ == state.isFinished_
        && boardState_ == state.boardState_;
}
}
