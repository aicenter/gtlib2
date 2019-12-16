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


#include <base/fogefg.h>
#include "domains/stratego.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2::domains {
unsigned int encodeMoveObservation(const int startPos, const int endPos,
                                   const CellState startCell, const CellState endCell) {
    // 30 bits total
    // max sizes of stratego boards are 10x10 = 100, so pos < 7 bits = 128
    unsigned int res = startPos << 23; // 7 bits
    res = res | (endPos << 16); // 7 bits
    res = res | (startCell << 8); // 8 bits
    res = res | endCell; // 8 bits
    return res;
}

vector<Rank> getMovableRanks(const vector<Rank> &figures) {
    vector<Rank> v;
    copy_if(figures.begin(),
            figures.end(),
            back_inserter(v),
            [&](Rank f) { return f != BOMB && f != FLAG; });
    return v;
}

enum ObservationType {
    SetupObs, MoveObs, EmptyObs
};

struct decodedObservation {
    decodedObservation(unsigned int startPos, unsigned int endPos,
                       unsigned int startCell, unsigned int endCell) :
        endCell(endCell), startCell(startCell), endPos(endPos), startPos(startPos),
        type(MoveObs) {};
    decodedObservation(unsigned int setupRank, unsigned int setupPos, unsigned int setupPlayerID) :
        setupPos(setupPos), setupRank(setupRank), setupPlayerID(setupPlayerID), type(SetupObs) {};
    decodedObservation() : type(EmptyObs) {};

    const ObservationType type;
    const unsigned int endCell = 0;
    const unsigned int startCell = 0;
    const unsigned int endPos = 0;
    const unsigned int startPos = 0;
    const unsigned int setupPos = 0;
    const unsigned int setupRank = 0;
    const unsigned int setupPlayerID = 0;
};

decodedObservation decodeStrategoObservation(const unsigned int obsid) {
    const unsigned int size8 = 255, size7 = 127;
    const int obstype = (obsid >> 30);
    if (obstype == 0) {
        const unsigned int endcell = obsid & size8;
        const unsigned int startcell = (obsid >> 8) & size8;
        const unsigned int endpos = (obsid >> 16) & size7;
        const unsigned int startpos = (obsid >> 23) & size7;
        return decodedObservation(startpos, endpos, startcell, endcell);
    }
    if (obstype == 1) {
        const unsigned int pos = obsid & size8;
        const unsigned int rank = (obsid >> 14) & size7;
        const unsigned int playerID = (obsid >> 28) & 3;
        return decodedObservation(rank, pos, playerID);
    }
    return decodedObservation();
}

// startPos/endPos < 512
unsigned int
encodeSetupObservation(const Rank rank, const unsigned int pos, const unsigned int playerID) {
    // 32 bits total
    //setupId up to 28 bits
    unsigned int res = playerID << 28;
    res = res | (1 << 30);
    res = res | (rank << 14); // 14 bits
    res = res | pos; // 14 bits
    return res;
}

int maxMovesWithoutAttack(const int height, const int width) {
    return (2 * height + 2 * width) * 4;
}

bool isPlayers(const CellState cell, const Player player) {
    if (cell == LAKE || cell == EMPTY) return false;
    return ((player == Player(0)) && (cell < 128)) || ((player == Player(1)) && (cell >= 128));
}

CellState createCell(const Rank figure, const Player player) {
    return player == Player(0) ? figure : figure + 128;
}

bool isSamePlayer(const CellState cell1, const CellState cell2) {
    if (cell1 == EMPTY || cell2 == EMPTY || cell1 == LAKE || cell2 == LAKE) return false;
    return (isPlayers(cell1, Player(0)) && isPlayers(cell2, Player(0)))
        || (isPlayers(cell1, Player(1)) && isPlayers(cell2, Player(1)));
}

Rank getRank(const CellState cell) {
    return cell < 128 ? cell : cell - 128;
}

bool isFigureSlain(const CellState attacker, const CellState defender) {
    assert(attacker != EMPTY && attacker != LAKE && defender != LAKE);
    if (defender == EMPTY) return true;

    const char rank1 = getRank(attacker);
    const char rank2 = getRank(defender);
    if (rank2 == BOMB) return rank1 == SAPPER;
    if (rank1 == SPY && rank2 == MARSHALL) return true;
    return rank1 > rank2;
}

bool StrategoSetupAction::operator==(const Action &that) const {
    if (typeid(that) == typeid(*this)) {
        const auto rhsAction = dynamic_cast<const StrategoSetupAction *>(&that);
        return (figureRank_ == rhsAction->figureRank_ && boardID_ == rhsAction->boardID_);
    }
    return false;
}

string StrategoSetupAction::toString() const {
    return "Position " + to_string(boardID_) + " was occupied with " + figureRank_;
}

bool StrategoMoveAction::operator==(const Action &that) const {
    if (typeid(that) == typeid(*this)) {
        const auto rhsAction = dynamic_cast<const StrategoMoveAction *>(&that);
        return (startPos_ == rhsAction->startPos_) && (endPos_ == rhsAction->endPos_);
    }
    return false;
}

string StrategoMoveAction::toString() const {
    return "move: (" + to_string(startPos_ % boardWidth_) + "," + to_string(startPos_ / boardWidth_)
        + ") -> (" + to_string(endPos_ % boardWidth_) + "," + to_string(endPos_ / boardWidth_)
        + ")";
}

vector<CellState> StrategoSettings::generateEmptyBoard() const {
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

StrategoSetupObservation::StrategoSetupObservation(const Rank figureRank,
                                                   const int boardID,
                                                   const int playerID)
    : Observation(), figureRank_(figureRank), boardID_(boardID), playerID_(playerID) {
    id_ = encodeSetupObservation(figureRank, boardID, playerID_);
}

StrategoMoveObservation::StrategoMoveObservation(const int startPos, const int endPos,
                                                 const Rank startCell, const Rank endCell)
    : Observation(), startPos_(startPos), endPos_(endPos), startCell_(startCell),
      endCell_(endCell) {
    assert(startPos + endPos > 0);
    id_ = encodeMoveObservation(startPos_, endPos_, startCell_, endCell_);
}

unsigned long fact(unsigned long n) {
    if (n == 0) return 1;
    if (n == 1) return 1;
    return n * fact(n - 1);
}

StrategoDomain::StrategoDomain(const StrategoSettings settings) :
    Domain(
        maxMovesWithoutAttack(settings.boardHeight, settings.boardWidth)
            * settings.figures.size() * 2, 2, true,
        make_shared<Action>(), make_shared<Observation>()),
    emptyBoard_(settings.generateEmptyBoard()), startFigures_(settings.figures),
    boardWidth_(settings.boardWidth), boardHeight_(settings.boardHeight) {
    assert(boardHeight_ * boardWidth_ > 1);
    const auto newState = make_shared<StrategoState>(this, emptyBoard_, Setup, 0, 0);
    newState->setFiguresToPlace(startFigures_);
    Outcome outcome(newState, {noObservation_, noObservation_}, noObservation_, {0.0, 0.0});
    maxUtility_ = 1.0;
    rootStatesDistribution_.emplace_back(OutcomeEntry(outcome));
}

string StrategoDomain::getInfo() const {
    return "Stratego game with " + to_string(emptyBoard_.size()) + " cells";
}

unique_ptr<StrategoDomain> StrategoDomain::STRAT4x4() {
    return make_unique<StrategoDomain>(StrategoSettings{
        4, 4, {{1, 1, 2, 2}}, vector<Rank>{'3', '2', '2', '1'}
    });
}

unique_ptr<StrategoDomain> StrategoDomain::STRAT3x3() {
    return make_unique<StrategoDomain>(StrategoSettings(3, 3, {{1, 1, 1, 1}}, {'1', '2', '3'}));
}

unique_ptr<StrategoDomain> StrategoDomain::STRAT3x2() {
    return make_unique<StrategoDomain>(StrategoSettings(3, 2, {}, {'1', '2'}));
}

unique_ptr<StrategoDomain> StrategoDomain::STRAT2x2() {
    return make_unique<StrategoDomain>(StrategoSettings(2, 2, {}, {'1', '2'}));
}


unsigned long StrategoDomain::inversePosition(const unsigned long pos) const {
    return boardHeight_ * boardWidth_ - pos - 1;
}

bool canMoveUp(const int i, const vector<CellState> &board, const int height, const int width) {
    return (height > 1)
        && ((i + 1) > width)
        && !isSamePlayer(board[i], board[i - width])
        && (board[i - width] != LAKE);
}

bool canMoveDown(const int i, const vector<CellState> &board, const int height, const int width) {
    return ((height > 1)
        && (board.size() - (i + 1) >= width)
        && !isSamePlayer(board[i], board[i + width])
        && (board[i + width] != LAKE));
}

bool canMoveLeft(const int i, const vector<CellState> &board, const int width) {
    return ((width > 1)
        && ((i + 1) % width != 1)
        && !isSamePlayer(board[i], board[i - 1])
        && (board[i - 1] != LAKE));
}

bool canMoveRight(const int i, const vector<CellState> &board, const int width) {
    return ((width > 1)
        && ((i + 1) % width != 0)
        && !isSamePlayer(board[i], board[i + 1])
        && (board[i + 1] != LAKE));
}

vector<Rank> distinctRanks(vector<Rank> v) {
    v.erase(unique(v.begin(), v.end()), v.end());
    return v;
}

unsigned long StrategoState::countAvailableActionsFor(const Player player) const {
    if (gameState_ == Setup)
        return distinctRanks(player == 0 ? remainingFiguresToPlace_[0]
                                         : remainingFiguresToPlace_[1]).size();

    const int height = domain_->boardHeight_;
    const int width = domain_->boardWidth_;
    int count = 0;
    //todo: add scouts (move to more than one field)
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

shared_ptr<Action> StrategoState::getActionByID(const Player player, const ActionId action) const {
    if (gameState_ == Setup) {
        auto t = distinctRanks(player == 0 ? remainingFiguresToPlace_[0]
                                           : remainingFiguresToPlace_[1]);
        return make_shared<StrategoSetupAction>(action, t[action], boardIDToPlace_);
    }
    const int height = domain_->boardHeight_;
    const int width = domain_->boardWidth_;
    int id = 0;
    //todo: add scouts (move to more than one field)
    for (int i = 0; i < boardState_.size(); i++) {
        if (!isPlayers(boardState_[i], player)
            || getRank(boardState_[i]) == BOMB || getRank(boardState_[i]) == FLAG)
            continue;

        if (canMoveUp(i, boardState_, height, width)) {
            if (id == action)
                return make_shared<StrategoMoveAction>(id, i, i - width, width);
            ++id;
        }
        if (canMoveRight(i, boardState_, width)) {
            if (id == action)
                return make_shared<StrategoMoveAction>(id, i, i + 1, width);
            ++id;
        }
        if (canMoveLeft(i, boardState_, width)) {
            if (id == action)
                return make_shared<StrategoMoveAction>(id, i, i - 1, width);
            ++id;
        }
        if (canMoveDown(i, boardState_, height, width)) {
            if (id == action)
                return make_shared<StrategoMoveAction>(id, i, i + width, width);
            ++id;
        }
    }

    unreachable("action id out of range!");
}

vector<shared_ptr<Action>> StrategoState::getAvailableActionsFor(const Player player) const {
    vector<shared_ptr<Action>> actions;
    int id = 0;
    if (gameState_ == Setup) {
        auto ranks = distinctRanks(player == 0 ? remainingFiguresToPlace_[0]
                                               : remainingFiguresToPlace_[1]);
        for (auto rank : ranks) {
            actions.push_back(make_shared<StrategoSetupAction>(id++, rank, boardIDToPlace_));
        }
        return actions;
    }

    const int height = domain_->boardHeight_;
    const int width = domain_->boardWidth_;
    //todo: add scouts (move to more than one field)
    for (int i = 0; i < boardState_.size(); i++) {
        if (!isPlayers(boardState_[i], player)
            || getRank(boardState_[i]) == BOMB || getRank(boardState_[i]) == FLAG)
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
    if (gameState_ == Finished) return {};
    return {currentPlayer_};
}

string StrategoState::toString() const {
    const int width = domain_->boardWidth_;
    const int height = domain_->boardHeight_;
    string result =
        "Pl: " + to_string(currentPlayer_) + "\nNo attacks: " + to_string(noAttackCounter_) + "\n";
    for (int i = 0; i < height; i++) {
        result += "\n";
        for (int j = 0; j < width; j++) {
            CellState figure = boardState_[width * i + j];
            if (figure == EMPTY) result += "__";
            else if (figure == LAKE) result += "LL";
            else {
                result += isPlayers(figure, 0) ? '0' : '1';
                result += getRank(figure);
            }
            result += ' ';
        }
    }
    return result;
}

OutcomeDistribution
StrategoState::performSetupAction(const vector<shared_ptr<Action>> &actions) const {
    const StrategoSetupAction action = dynamic_cast<const StrategoSetupAction &>(
        *actions[currentPlayer_]);
    vector<CellState> newBoard = boardState_;

    newBoard[currentPlayer_ == 0 ? action.boardID_ : domain_->inversePosition(action.boardID_)] =
        createCell(action.figureRank_, currentPlayer_);
    shared_ptr<StrategoState> newState;

    const bool transitionToMovePhase =
        remainingFiguresToPlace_[1].size() == 1 && currentPlayer_ == 1;

    if (transitionToMovePhase)
        newState = make_shared<StrategoState>(domain_, newBoard, Playing, 0, 0);
    else {
        newState = make_shared<StrategoState>(
            domain_, newBoard, Setup, opponent(currentPlayer_), 0);
        newState->boardIDToPlace_ = boardIDToPlace_;
        newState->remainingFiguresToPlace_[0] = remainingFiguresToPlace_[0];
        newState->remainingFiguresToPlace_[1] = remainingFiguresToPlace_[1];
        newState->remainingFiguresToPlace_[currentPlayer_].erase(std::find(
            newState->remainingFiguresToPlace_[currentPlayer_].begin(),
            newState->remainingFiguresToPlace_[currentPlayer_].end(), action.figureRank_));
        if (currentPlayer_ == 1)
            newState->boardIDToPlace_++; // Both players made their placement turns
    }

    const auto noObs = domain_->getNoObservation();
    const auto obs0 = make_shared<StrategoSetupObservation>(
        action.figureRank_, action.boardID_, currentPlayer_);
    const auto obs1 = make_shared<StrategoSetupObservation>(
        0, action.boardID_, currentPlayer_);

    const auto newOutcome = Outcome(newState,
                                    {currentPlayer_ == 0 ? obs0 : obs1,
                                     currentPlayer_ == 0 ? obs1 : obs0},
                                    noObs, {0, 0});
    return OutcomeDistribution{OutcomeEntry(newOutcome)};
}

vector<CellState> updateBoard(const vector<CellState> &oldBoard, const int start, const int end) {
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

pair<bool, bool> checkOnlyOneMovablePieceRemains(const vector<CellState> &newBoard,
                                                 int height, int width) {
    CellState pl0fig = EMPTY, pl1fig = EMPTY;
    int pl0MovableCounter = 0, pl1MovableCounter = 0, immovableCounter = 0;
    for (int i = 0; i < newBoard.size(); i++) {
        auto f = newBoard[i];
        if (f == EMPTY || f == LAKE) continue;
        if (getRank(f) == BOMB || getRank(f) == FLAG) {
            immovableCounter++;
            continue;
        }
        if (!canMoveUp(i, newBoard, height, width) &&
            !canMoveDown(i, newBoard, height, width) &&
            !canMoveLeft(i, newBoard, width) &&
            !canMoveRight(i, newBoard, width)) {
            immovableCounter++;
            continue;
        }
        if (isPlayers(f, 0)) {
            pl0MovableCounter++;
            pl0fig = pl0fig > getRank(f) ? pl0fig : getRank(f);
        }
        if (isPlayers(f, 1)) {
            pl1MovableCounter++;
            pl1fig = pl1fig > getRank(f) ? pl1fig : getRank(f);
        }
    }

    if (pl0MovableCounter == 0 || pl1MovableCounter == 0)
        return make_pair(pl1MovableCounter == 0, pl0MovableCounter == 0);
    if (immovableCounter == 0 && pl0MovableCounter == 1 && pl1MovableCounter == 1)
        return make_pair(pl0fig >= pl1fig, pl1fig >= pl0fig);
    return make_pair(false, false);
}

OutcomeDistribution
StrategoState::performMoveAction(const vector<shared_ptr<Action>> &actions) const {
    StrategoMoveAction action = dynamic_cast<StrategoMoveAction &>(*actions[currentPlayer_]);
    const int height = domain_->boardHeight_;
    const int width = domain_->boardWidth_;
    const CellState &startCell = boardState_.at(action.startPos_);
    const CellState &endCell = boardState_.at(action.endPos_);
    assert(startCell != EMPTY && startCell != LAKE);

    vector<CellState> newBoard = updateBoard(boardState_, action.startPos_, action.endPos_);
    bool pl0won = false, pl1won = false;

    if (getRank(endCell) == FLAG) { // Flag captured!
        if (currentPlayer_ == 0) pl0won = true;
        else pl1won = true;
    } else tie(pl0won, pl1won) = checkOnlyOneMovablePieceRemains(newBoard, height, width);

    const vector<double> newRewards = {(pl0won ? 1.0 : 0.0) + (pl1won ? (-1.0) : 0.0),
                                       (pl1won ? 1.0 : 0.0) + (pl0won ? (-1.0) : 0.0)};

    shared_ptr<StrategoMoveObservation>
        obs = make_shared<StrategoMoveObservation>(action.startPos_, action.endPos_,
                                                   endCell == EMPTY ? EMPTY
                                                                    : startCell, // do not reveal rank if moving to empty cell
                                                   endCell);

    if (endCell == EMPTY && (noAttackCounter_ == maxMovesWithoutAttack(height, width))) {
        const auto newState =
            make_shared<StrategoState>(domain_, newBoard, Finished, opponent(currentPlayer_), 0);
        const auto newOutcome = Outcome(newState, {obs, obs}, obs, newRewards);
        return OutcomeDistribution{OutcomeEntry(newOutcome)};
    }

    const auto newState = make_shared<StrategoState>(domain_,
                                                     newBoard,
                                                     (pl0won || pl1won) ? Finished : Playing,
                                                     opponent(currentPlayer_),
                                                     (endCell == EMPTY) ? noAttackCounter_ + 1 : 0);
    const auto newOutcome = Outcome(newState, {obs, obs}, obs, newRewards);
    return OutcomeDistribution{OutcomeEntry(newOutcome)};
}

OutcomeDistribution StrategoState::performActions(const vector<shared_ptr<Action>> &actions) const {
    if (gameState_ == Setup) return performSetupAction(actions);
    return performMoveAction(actions);
}

bool StrategoState::isTerminal() const { return gameState_ == Finished; }

bool StrategoState::operator==(const State &rhs) const {
    const auto state = dynamic_cast<const StrategoState &>(rhs);
    return hash_ == state.hash_
        && currentPlayer_ == state.currentPlayer_
        && gameState_ == state.gameState_
        && boardState_ == state.boardState_;
}

unsigned int backtrackPosition(const vector<ActionObservationIds> &aoids,
                               const unsigned int endPos, long i) {
    auto startPos = endPos;
    for (--i; i > 0; --i) {
        const auto currObs = domains::decodeStrategoObservation(aoids[i].observation);
        if (currObs.type != MoveObs) break;
        if (currObs.endPos == startPos && (currObs.endCell == domains::EMPTY
            || domains::isFigureSlain(currObs.startCell, currObs.endCell)))
            startPos = currObs.startPos;
    }
    return startPos;
}

bool StrategoDomain::updateConstraints(const shared_ptr<AOH> &currentInfoset,
                                       long &startIndex, ConstraintsMap &revealedFigures) const {
    bool isValid = false;
    const auto currentAOids = currentInfoset->getAOids();
    const auto currentPlayer = currentInfoset->getPlayer();

    for (unsigned long i = startIndex + 1; i < currentAOids.size(); i++) {
        if (currentAOids[i].observation == getNoObservation()->getId())
            continue;
        const auto currentObservation = decodeStrategoObservation(currentAOids[i].observation);
        if (currentObservation.type != MoveObs) continue;

        isValid = true;

        if (currentObservation.endCell == EMPTY) { // move observation
            const unsigned long
                pos = currentPlayer == 0 ? inversePosition(currentObservation.startPos)
                                         : currentObservation.startPos;
            if (pos < startFigures_.size() && revealedFigures.find(pos) == revealedFigures.end())
                revealedFigures[pos] = make_shared<StrategoConstraint>(true);
            continue;
        }

        // attack observation
        const bool isOpponentsTurn = currentAOids[i].action == getNoAction()->getId();
        unsigned long pos = backtrackPosition(currentAOids,
                                              isOpponentsTurn ? currentObservation.startPos
                                                              : currentObservation.endPos,
                                              i);
        if (currentPlayer == 0)
            // position of pl1's figure is on the other side of the board
            pos = inversePosition(pos);

        const Rank val = getRank(isOpponentsTurn ? currentObservation.startCell
                                                 : currentObservation.endCell);
        if (revealedFigures.find(pos) == revealedFigures.end()) {
            revealedFigures[pos] = make_shared<StrategoConstraint>(val);
        } else {
            auto currentConstraint = dynamic_cast<StrategoConstraint *>(revealedFigures[pos].get());
            if (currentConstraint->revealedRank == EMPTY)
                currentConstraint->revealedRank = val;
            else if (currentConstraint->revealedRank != val) unreachable("Incorrect revealing");
        }
        startIndex = i;
    }
    return isValid;
}

void StrategoDomain::nodeGenerationTerminalPhase(const vector<ActionObservationIds> &aoids,
                                                 const shared_ptr<EFGNode> &node,
                                                 const EFGNodeCallback &newNodeCallback) const {
    auto currentNode = node;
    for (size_t i = startFigures_.size() * 2 + 1; i < aoids.size(); i++) {
        if (currentNode->getAOHInfSet()->getAOids() == aoids) { //simulation finished
            newNodeCallback(currentNode);
            break;
        }
        if (aoids[i].action != getNoAction()->getId()) {
            currentNode = currentNode->performAction(currentNode->getActionByID(aoids[i].action));
            continue;
        }

        // opponents turn
        const auto currentObservation = domains::decodeStrategoObservation(aoids[i].observation);
        if (currentObservation.type != MoveObs)
            unreachable("wrong revealing");
        bool moveActionFound = false;
        for (const auto &a : currentNode->availableActions()) {
            const auto action = dynamic_pointer_cast<domains::StrategoMoveAction>(a);
            if (action->startPos_ == currentObservation.startPos
                && action->endPos_ == currentObservation.endPos) {
                moveActionFound = true;
                currentNode = currentNode->performAction(a);
                break;
            }
        }
        if (!moveActionFound)
            break; //same action not found => board setup does not fit
    }
}

void StrategoDomain::recursiveNodeGeneration(const shared_ptr<AOH> &currentInfoset,
                                             const shared_ptr<EFGNode> &node, const int depth,
                                             const vector<shared_ptr<StrategoConstraint>> &mask,
                                             const vector<Rank> &remaining,
                                             const BudgetType budgetType, int &counter,
                                             const EFGNodeCallback &newNodeCallback) const {
    if ((budgetType == BudgetIterations && counter <= 0)
        || (budgetType == BudgetTime && counter - int(clock()) / 1000 <= 0))
        return;
    const auto currentAOids = currentInfoset->getAOids();
    const auto currentPlayer = currentInfoset->getPlayer();
    if (depth == mask.size()) {
        if (budgetType == BudgetIterations) counter--;
        nodeGenerationTerminalPhase(currentAOids, node, newNodeCallback);
        return;
    }

    shared_ptr<EFGNode> currentNode = node;
    if (currentPlayer == 0)
        currentNode = currentNode->performAction(
            currentNode->getActionByID(currentAOids[2 * (depth + 1) - 1].action));

    if (mask[depth]->revealedRank != EMPTY) {
        for (const auto &a : currentNode->availableActions()) {
            if (dynamic_pointer_cast<domains::StrategoSetupAction>(a)->figureRank_
                != mask[depth]->revealedRank)
                continue;
            currentNode = currentNode->performAction(a);
            break;
        }
        if (currentPlayer == 1)
            currentNode = currentNode->performAction(
                currentNode->getActionByID(currentAOids[2 * (depth + 1)].action));

        recursiveNodeGeneration(currentInfoset, currentNode, depth + 1, mask, remaining,
                                budgetType, counter, newNodeCallback);
        return;
    }
    auto unrepeatedRanks = distinctRanks(remaining);
    for (auto rank : mask[depth]->moved ? getMovableRanks(unrepeatedRanks) : unrepeatedRanks) {
        auto newNode = currentNode;
        vector<Rank> newRemaining = remaining;
        bool found = false;
        for (const auto &a : currentNode->availableActions()) {
            if (dynamic_pointer_cast<domains::StrategoSetupAction>(a)->figureRank_ != rank)
                continue;
            found = true;
            newNode = newNode->performAction(a);
            newRemaining.erase(std::find(newRemaining.begin(), newRemaining.end(), rank));
            break;
        }

        if (!found) continue;
        if (currentPlayer == 1)
            newNode = newNode->performAction(
                newNode->getActionByID(currentAOids[2 * (depth + 1)].action));
        recursiveNodeGeneration(currentInfoset, newNode, depth + 1, mask, newRemaining,
                                budgetType, counter, newNodeCallback);
    }
}

void StrategoDomain::generateNodes(const shared_ptr<AOH> &currentInfoset,
                                   const ConstraintsMap &revealedInfo,
                                   const BudgetType budgetType,
                                   const int budget,
                                   const EFGNodeCallback &newNodeCallback) const {
    auto mask = vector<shared_ptr<StrategoConstraint>>(startFigures_.size());
    for (auto &i : mask) i = make_shared<StrategoConstraint>(Rank(EMPTY));

    vector<Rank> remaining = startFigures_;
    for (auto &[turn, constraint] : revealedInfo) {
        const auto currentConstraint = dynamic_cast<StrategoConstraint *>(constraint.get());
        if (currentConstraint->revealedRank != EMPTY) {
            const auto position = std::find(remaining.begin(), remaining.end(),
                                            currentConstraint->revealedRank);
            if (position != remaining.end())
                remaining.erase(position);
            else
                unreachable("Incorrect revealing");
        }
        mask[turn]->revealedRank = currentConstraint->revealedRank;
        mask[turn]->moved = currentConstraint->moved;
    }

    const auto root = createRootEFGNode(*this);
    int a = budgetType == BudgetIterations ? budget : int(clock()) / 1000 + budget;
    recursiveNodeGeneration(currentInfoset, root, 0, mask, remaining,
                            budgetType, a, newNodeCallback);
}
}
