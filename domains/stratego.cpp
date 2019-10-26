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
unsigned int encodeMoveObservation(int startPos, int endPos, CellState startCell, CellState endCell) {
    // 30 bits total
    // max sizes of stratego boards are 10x10 = 100, so pos < 7 bits = 128
    unsigned int res = startPos << 23; // 7 bits
    res = res | (endPos << 16); // 7 bits
    res = res | (startCell << 8); // 8 bits
    res = res | endCell; // 8 bits
    return res;
}

vector<Rank> getMovableRanks(vector<Rank> figures)
{
    vector<Rank> v;
    for (auto f: figures)
    {
        if (f != BOMB && f != FLAG) v.push_back(f);
    }
    return v;
}

vector<unsigned int> decodeStrategoObservation(unsigned int obsid) {
    unsigned int size8 = 255, size7 = 127;
    int obstype = (obsid>>30);
    if (obstype == 0) {
        unsigned int endcell = obsid & size8;
        unsigned int startcell = (obsid >> 8) & size8;
        unsigned int endpos = (obsid >> 16) & size7;
        unsigned int startpos = (obsid >> 23) & size7;

        return {startpos, endpos, startcell, endcell};
    }
    if (obstype == 1) {
        unsigned int pos = obsid & size8;
        unsigned int rank = (obsid >> 14) & size7;
        unsigned int playerID = (obsid >> 28) & 3;
        return {rank, pos, playerID};
    }
    return {};
}

// startpos/endpos < 512
unsigned int encodeSetupObservation(Rank rank, unsigned int pos, unsigned int playerID) {
    // 32 bits total
    //setupid up to 28 bits
    unsigned int res = playerID << 28;
    res = res | (1 << 30);
    res = res | (rank << 14); // 14 bits
    res = res | pos; // 14 bits
    return  res ;
}

int maxMovesWithoutAttack(int h, int w) {
    return (4 * h + 4 * w)*2;
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
        return (figureRank == rhsAction->figureRank && boardID == rhsAction->boardID);
    }
    return false;
}

string StrategoSetupAction::toString() const {
    return "Position " + to_string(boardID) + " was occupied with " + figureRank;
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


StrategoSetupObservation::StrategoSetupObservation(const Rank figureRank, const int boardID,  const int playerID)
    : Observation(), figureRank_(figureRank), boardID_(boardID), playerID_(playerID) {
    id_ = encodeSetupObservation(figureRank, boardID, playerID_);
}

StrategoMoveObservation::StrategoMoveObservation(const int startPos, const int endPos,
                                         const Rank startCell, const Rank endCell)
    : Observation(), startPos_(startPos), endPos_(endPos), startCell_(startCell), endCell_(endCell) {
    assert(startPos + endPos > 0);
    id_ = encodeMoveObservation(startPos_, endPos_, startCell_, endCell_);
}

vector<Rank> sortVector(vector<Rank> v) {
    sort(v.begin(), v.end());
    return v;
}

unsigned long fact(unsigned long n) {
    if (n == 0) return 1;
    if (n == 1) return 1;
    return n * fact(n - 1);
}

StrategoDomain::StrategoDomain(StrategoSettings settings) :
    Domain(maxMovesWithoutAttack(settings.boardHeight, settings.boardWidth)
               * settings.figures.size() * 2, 2, true,
                    make_shared<Action>(), make_shared<Observation>()),
    emptyBoard_(settings.generateBoard()),
    startFigures_(sortVector(settings.figures)),
    boardWidth_(settings.boardWidth),
    boardHeight_(settings.boardHeight) {
    assert(boardHeight_ * boardWidth_ > 1);
    const auto newState = make_shared<StrategoState>(this, emptyBoard_, true, false, 0, 0);
    newState->pl0FiguresToPlace_ = startFigures_;
    newState->pl1FiguresToPlace_ = startFigures_;
    Outcome outcome(newState, {noObservation_, noObservation_}, noObservation_, {0.0, 0.0});
    maxUtility_ = 1.0;
    rootStatesDistribution_.emplace_back(OutcomeEntry(outcome));
}

string StrategoDomain::getInfo() const {
    return "Stratego game with " + to_string(emptyBoard_.size()) + " cells";
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

vector<Rank> removeSimilar(const vector<Rank> & orig)
{
    auto t = orig;
    t.erase( unique( t.begin(), t.end() ), t.end() );
    return t;
}

unsigned long StrategoState::countAvailableActionsFor(Player player) const {
    const auto stratDomain = dynamic_cast<const StrategoDomain *>(domain_);
    if (isSetupState_) {
        return removeSimilar(player == 0 ? pl0FiguresToPlace_ : pl1FiguresToPlace_).size();//countDistinctPermutations(stratDomain->startFigures_);
    }
    int height = stratDomain->boardHeight_;
    int width = stratDomain->boardWidth_;
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

shared_ptr<Action> StrategoState::getActionByID(const Player player, ActionId action) const {
    const auto stratDomain = dynamic_cast<const StrategoDomain *>(domain_);
    if (isSetupState_) {
        auto t = removeSimilar(player == 0 ? pl0FiguresToPlace_ : pl1FiguresToPlace_);
        return make_shared<StrategoSetupAction>(action, t[action], boardIDToPlace_);
    }
    const int height = stratDomain->boardHeight_;
    const int width = stratDomain->boardWidth_;
    int id = 0;
    //todo: add scouts (move to more than one field)
    for (int i = 0; i < boardState_.size(); i++) {
        if (!isPlayers(boardState_[i], player)
            || getRank(boardState_[i]) == BOMB
            || getRank(boardState_[i]) == FLAG)
            continue;

        if (canMoveUp(i, boardState_, height, width)) {
            if (id == action)
                return make_shared<StrategoMoveAction>(id++, i, i - width, width);
            id++;
        }
        if (canMoveRight(i, boardState_, width)) {
            if (id == action)
                return make_shared<StrategoMoveAction>(id++, i, i + 1, width);
            id++;
        }
        if (canMoveLeft(i, boardState_, width)) {
            if (id == action)
                return make_shared<StrategoMoveAction>(id++, i, i - 1, width);
            id++;
        }
        if (canMoveDown(i, boardState_, height, width)) {
            if (id == action)
                return make_shared<StrategoMoveAction>(id++, i, i + width, width);
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
        auto t = removeSimilar(player == 0 ? pl0FiguresToPlace_ : pl1FiguresToPlace_);
        for (auto f : t)
        {
            actions.push_back(make_shared<StrategoSetupAction>(id++, f, boardIDToPlace_));
        }
        return actions;
    }
    //todo: add scouts (move to more than one field)
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
    //if (isSetupState_) return {0, 1};
    //    //else
    if (isFinished_) return {};
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
        action = dynamic_cast<StrategoSetupAction &>(*actions[currentPlayer_]);
    vector<CellState> board = boardState_;
    board[currentPlayer_ == 0 ? action.boardID : (board.size() - 1 - action.boardID)] = currentPlayer_ == 0 ?
                                                action.figureRank : createCell(action.figureRank, Player(1));
    shared_ptr<StrategoState> newState;
    if (pl1FiguresToPlace_.size() == 1 && currentPlayer_ == 1) {
            newState = make_shared<StrategoState>(stratDomain, board, false, false, 0, 0);
    }
    else {
        if (currentPlayer_ == 0)
        {
            newState = make_shared<StrategoState>(stratDomain, board, true, false, 1, 0);
            newState->pl0FiguresToPlace_ = pl0FiguresToPlace_;
            newState->pl1FiguresToPlace_ = pl1FiguresToPlace_;
            for (int i = 0; i < newState->pl0FiguresToPlace_.size(); i++) {
                if (newState->pl0FiguresToPlace_[i] == action.figureRank) {
                    newState->pl0FiguresToPlace_.erase(newState->pl0FiguresToPlace_.begin() + i);
                    break;
                }
            }
            newState->boardIDToPlace_ = boardIDToPlace_;
        }
        else
        {
            newState = make_shared<StrategoState>(stratDomain, board, true, false, 0, 0);
            newState->pl0FiguresToPlace_ = pl0FiguresToPlace_;
            newState->pl1FiguresToPlace_ = pl1FiguresToPlace_;
            for (int i = 0; i < newState->pl1FiguresToPlace_.size(); i++) {
                if (newState->pl1FiguresToPlace_[i] == action.figureRank) {
                    newState->pl1FiguresToPlace_.erase(newState->pl1FiguresToPlace_.begin() + i);
                    break;
                }
            }
            newState->boardIDToPlace_ = boardIDToPlace_ + 1;
        }
    }
    const auto &noObs = stratDomain->getNoObservation();
    const auto obs0 =  make_shared<StrategoSetupObservation>(action.figureRank, action.boardID, currentPlayer_);
    const auto obs1 =  make_shared<StrategoSetupObservation>(0, action.boardID, currentPlayer_);
    const auto newOutcome = Outcome(newState,
        {currentPlayer_ == 0 ? obs0 : obs1, currentPlayer_ == 0 ? obs1 : obs0},
        noObs, {0, 0});

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

pair<bool, bool> checkOnlyOneMovablePieceRemains(const vector<CellState> &newBoard, int height, int width) {
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
        tie(pl0won, pl1won) = checkOnlyOneMovablePieceRemains(newBoard, stratDomain->boardHeight_, stratDomain->boardWidth_);
    }

    const vector<double> newRewards = {(pl0won ? 1.0 : 0.0) + (pl1won ? (-1.0) : 0.0),
                                       (pl1won ? 1.0 : 0.0) + (pl0won ? (-1.0) : 0.0)};

    shared_ptr<StrategoMoveObservation> obs = make_shared<StrategoMoveObservation>(
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

unsigned int backtrackPosition(const vector<ActionObservationIds> & aoids, unsigned int endpos, long  i) {
    auto startpos = endpos;
    while(i > 0) {
        i--;
        const auto currObs = domains::decodeStrategoObservation(aoids[i].observation);
        if (currObs.size() != 4) break;
        if (currObs[1] == startpos) {
            if (currObs[3] != domains::EMPTY && !domains::isFigureSlain(currObs[2], currObs[3]))
                continue;
            startpos = currObs[0];
        }
    }
    return startpos;
}



bool StrategoDomain::proceedAOIDs(const shared_ptr<AOH> & currentInfoset,
    long & startIndex, ConstraintsMap & revealedFigures) const
{
    bool newFigureRevealed = false;
    for (unsigned long i = startIndex + 1; i < currentInfoset->getAOids().size(); i++) {
        if (currentInfoset->getAOids()[i].observation == getNoObservation()->getId()) break;
        const auto res = decodeStrategoObservation(currentInfoset->getAOids()[i].observation);
        if (res.size() != 4) continue;
        if (res[3] != EMPTY) {
            unsigned long pos =
                backtrackPosition(currentInfoset->getAOids(), (currentInfoset->getAOids()[i].action == getNoAction()->getId()) ? res[0] : res[1], i);
            if (currentInfoset->getPlayer() == 0) pos = boardHeight_ * boardWidth_ - pos - 1;
            const Rank val = getRank((currentInfoset->getAOids()[i].action == getNoAction()->getId()) ? res[2] : res[3]);
            if (revealedFigures.find(pos) == revealedFigures.end()) {
                revealedFigures[pos] = make_shared<StrategoRevealedInfo>(val);
            } else {
                auto a = dynamic_cast<StrategoRevealedInfo *>(revealedFigures[pos].get());
                if (a->revealedRank == EMPTY)
                    a->revealedRank = val;
                else if (a->revealedRank != val)
                    unreachable("Incorrect revealing");
            }
            startIndex = i;
            newFigureRevealed = true;
        }
        else {
            const unsigned long pos = currentInfoset->getPlayer() == 0 ? boardHeight_ * boardWidth_ - res[0] - 1 : res[0];
            if (pos < startFigures_.size() && revealedFigures.find(pos) == revealedFigures.end())
                revealedFigures[pos] = make_shared<StrategoRevealedInfo>(true);
        }
    }
    return newFigureRevealed;
}

void StrategoDomain::simulateMoves(const vector<ActionObservationIds> & aoids,
    const shared_ptr<EFGNode> node, const std::function<double(const shared_ptr<EFGNode> &)> & newNodeCallback) const{
    auto currentNode = node;
    for (int i = startFigures_.size()*2+1; i < aoids.size(); i++) {
        if (currentNode->getAOHInfSet()->getAOids() == aoids) {
            newNodeCallback(currentNode);
            break;
        }
        if (aoids[i].action == getNoAction()->getId()) {
            const auto currentObservation = domains::decodeStrategoObservation(aoids[i].observation);
            bool moveActionFound = false;
            for (const auto& a : currentNode->availableActions()) {
                const auto action = dynamic_pointer_cast<domains::StrategoMoveAction>(a);
                if (action->startPos == currentObservation[0] && action->endPos == currentObservation[1]) {
                    moveActionFound = true;
                    currentNode = currentNode->performAction(a);
                    break;
                }
            }
            if (!moveActionFound)
                break; //same action not found => board setup does not fit
        }
        else
            currentNode = currentNode->performAction(currentNode->getActionByID(aoids[i].action));
    }
}

void StrategoDomain::recursiveNodeGeneration(const shared_ptr<AOH> & currentInfoset,
                                             const shared_ptr<EFGNode> & node, int depth,
                                             const vector<shared_ptr<StrategoRevealedInfo>> & mask,
                                             const vector<Rank>& remaining, int & counter,
                                             const std::function<double(const shared_ptr<EFGNode> &)>& newNodeCallback) const {
    shared_ptr<EFGNode> currentNode = node;
    if (counter <= 0) return;
    if (depth == mask.size()) {
        counter--;
        simulateMoves(currentInfoset->getAOids(), currentNode, newNodeCallback);
        return;
    }
    if (currentInfoset->getPlayer() == 0) currentNode = currentNode->performAction(currentNode->getActionByID(currentInfoset->getAOids()[1 + 2 * depth].action));
    if (mask[depth]->revealedRank != EMPTY) {
        for (const auto& a : currentNode->availableActions()) {
            const auto action = dynamic_pointer_cast<domains::StrategoSetupAction>(a);
            if (action->figureRank == mask[depth]->revealedRank) {
                currentNode = currentNode->performAction(a);
                break;
            }
        }
        if (currentInfoset->getPlayer() == 1)
            currentNode = currentNode->performAction(currentNode->getActionByID(currentInfoset->getAOids()[1 + 2 * depth + 1].action));
        recursiveNodeGeneration(currentInfoset, currentNode, depth + 1, mask, remaining, counter, newNodeCallback);
        return;
    }
    for (auto rank : mask[depth]->moved ? getMovableRanks(remaining) : remaining) {
        auto newNode = currentNode;
        vector<Rank> newRemaining = remaining;
        bool found = false;
        for (const auto& a : currentNode->availableActions()) {
            const auto action = dynamic_pointer_cast<domains::StrategoSetupAction>(a);
            if (action->figureRank == rank) {
                found = true;
                newNode = newNode->performAction(a);
                newRemaining.erase(std::find(newRemaining.begin(), newRemaining.end(), rank));
                break;
            }
        }
        if (!found)
            continue;
        if (currentInfoset->getPlayer() == 1) newNode = newNode->performAction(newNode->getActionByID(currentInfoset->getAOids()[1 + 2*depth + 1].action));
        recursiveNodeGeneration(currentInfoset, newNode, depth + 1, mask, newRemaining, counter, newNodeCallback);
    }
}

void StrategoDomain::generateNodes(const shared_ptr<AOH> & currentInfoset, const ConstraintsMap & revealedFigures,
                                        const int max, const std::function<double(const shared_ptr<EFGNode> &)> & newNodeCallback) const
{
    vector<shared_ptr<StrategoRevealedInfo>> mask = vector<shared_ptr<StrategoRevealedInfo>>(startFigures_.size());
    for (int i = 0 ; i < mask.size(); i++)
        mask[i] = make_shared<StrategoRevealedInfo>(Rank(EMPTY));
    vector<Rank> remaining = startFigures_;
    for (auto p : revealedFigures)
    {
        auto currentConstraints = dynamic_cast<StrategoRevealedInfo*>(p.second.get());
        if (currentConstraints->revealedRank != EMPTY) {
            auto position = std::find(remaining.begin(), remaining.end(), currentConstraints->revealedRank);
            if (position != remaining.end())
                remaining.erase(position);
            else unreachable("Incorrect revealing");
        }
        mask[p.first]->revealedRank = currentConstraints->revealedRank;
        mask[p.first]->moved = currentConstraints->moved;
    }
    const auto root = createRootEFGNode(*this);
    int a = max;
    recursiveNodeGeneration(currentInfoset, root, 0, mask, remaining, a, newNodeCallback);
}
}
