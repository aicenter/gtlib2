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
    unsigned int encodeAction(int startPos, int endPos, Rank startRank, Rank endRank)
    {
        return (startPos << 23) | (endPos << 14) | (startRank << 7) | endRank; // 32 bits total
    }

    int maxMovesWithoutAttack(int h, int w)
    {
        return (2*h + 2*w - 4)*2;
    }

    bool isPlayers(CellState figure, Player player)
    {
        if (figure == 'L')  return false;
        if  (figure == ' ') return false;
        return (((player == 0) && (figure < 128)) || ((player == 1) && (figure >= 128))) && (figure != 'L');
    }

    CellState getCellState(Rank figure, Player player) {
        return player == 0 ? figure : figure + 128;
    }

    bool isSamePlayer(CellState figure1, CellState figure2)
    {
        if (figure2 == ' ') return false;
        return (isPlayers(figure1, 0) && isPlayers(figure2, 0)) || (isPlayers(figure1, 1) && isPlayers(figure2, 1)) ;
    }

    Rank getRank(CellState figure)
    {
        return figure < 128 ? figure : figure - 128;
    }

    bool isFigureSlain(CellState figure1, CellState figure2)
    {
        char f1 = getRank(figure1);
        char f2 = getRank(figure2);
        if (f1 == ' ') return true;
        if (f2 == 'B') return f1 == '2'; // sapper rank
        if (f1 == '0' && f2 == '9') return true; // 0 - Spy (min rank), 9 - Marshal (max Rank)

        return f1 > f2;
    }

    bool StrategoSetupAction::operator==(const Action &that) const {
        if (typeid(that) == typeid(*this)) {
            const auto rhsAction = dynamic_cast<const StrategoSetupAction *>(&that);
            return (figuresSetup == rhsAction->figuresSetup);
        }
        return false;
    }

    string StrategoSetupAction::toString() const
    {
        string out;
        for (auto & i : figuresSetup)
        {
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

        return "move: (" + to_string(startPos%boardWidth_) + "," + to_string(startPos/boardWidth_) + ") -> (" + to_string(endPos%boardWidth_) + "," + to_string(endPos/boardWidth_) + ")";
    }

    vector<CellState> StrategoSettings::generateBoard() {
        vector<CellState> board(getBoardSize());
        fill(board.begin(), board.end(), ' ');
        for (auto & lake : lakes) {
            for (int i = 0; i < lake.height; i++)
            {
                for (int j = 0; j < lake.width; j++)
                {
                    board[(lake.y + i)*boardWidth + lake.x + j] = 'L';
                }
            }
        }
        return board;
    }

    StrategoObservation::StrategoObservation(const int startPos, const int endPos, const Rank startRank, const Rank endRank)
            : Observation(),
              startPos_(startPos),
              endPos_(endPos),
              startRank_(startRank),
              endRank_(endRank) {

        assert(startPos+endPos > 0);

        id_ = encodeAction(startPos_, endPos_, startRank_, endRank_);
    }


    StrategoDomain::StrategoDomain(StrategoSettings settings) :
            Domain(maxMovesWithoutAttack(settings.boardHeight, settings.boardWidth)*settings.figures.size()*2,
                    2, true, make_shared<Action>(),
                   make_shared<StrategoObservation>()),
            emptyBoard_(settings.generateBoard()),
            startFigures_(settings.figures),
            boardWidth_(settings.boardWidth),
            boardHeight_(settings.boardHeight){
        assert(boardHeight_*boardWidth_ > 1);
        const auto newState = make_shared<StrategoState>(this, emptyBoard_, true, false, 0, 0);
        shared_ptr<StrategoObservation> obs = make_shared<StrategoObservation>();
        Outcome outcome(newState, {obs, obs}, obs, {0.0, 0.0});
        maxUtility_ = 1.0;
        rootStatesDistribution_.emplace_back(OutcomeEntry(outcome));
    }

    string StrategoDomain::getInfo() const {
        return "Stratego game with " + to_string(emptyBoard_.size()) + " cells";
    }

    int fact(int n)
    {
        if (n == 0) return 1;
        if (n == 1) return 1;
        return n*fact(n-1);
    }

    bool canMoveUp(int i, const vector<CellState> &board, int height, int width)
    {
        return (height > 1) && ((i + 1) > width) && !isSamePlayer(board[i], board[i - width]) &&
               (board[i - width] != 'L');
    }

    bool canMoveDown(int i, const vector<CellState> &board, int height, int width)
    {
        return ((height > 1) && (board.size() - (i+1) >= width) && !isSamePlayer(board[i], board[i + width]) && (board[i + width] != 'L'));
    }

    bool canMoveLeft(int i, const vector<CellState> &board, int width)
    {
        return ((width > 1) && ((i+1) % width != 1) && !isSamePlayer(board[i], board[i - 1]) && (board[i - 1] != 'L'));
    }

    bool canMoveRight(int i, const vector<CellState> &board, int width)
    {
        return ((width > 1) && ((i+1) % width != 0) && !isSamePlayer(board[i], board[i + 1]) && (board[i + 1] != 'L'));
    }

    unsigned long StrategoState::countAvailableActionsFor(Player player) const {
        const auto stratDomain = dynamic_cast<const StrategoDomain *>(domain_);
        int height = stratDomain->boardHeight_;
        int width = stratDomain->boardWidth_;
        if (isSetupState_)
        {
            return fact(stratDomain->startFigures_.size());
        }
        int count = 0;
        for (int i = 0; i < boardState_.size(); i++) {
            if (!isPlayers(boardState_[i], player) || getRank(boardState_[i]) == 'B' || getRank(boardState_[i]) == 'F') continue;
            if (canMoveUp(i, boardState_, height, width)) count++;
            if (canMoveRight(i, boardState_, width)) count++;
            if (canMoveLeft(i, boardState_, width)) count++;
            if (canMoveDown(i, boardState_, height, width)) count++;
        }
        return count;
    }

    shared_ptr<Action> StrategoState::getActionByID(const Player player, const int actionID) const {
        const auto stratDomain = dynamic_cast<const StrategoDomain *>(domain_);
        int id = 0;
        int height = stratDomain->boardHeight_;
        int width = stratDomain->boardWidth_;
        if (isSetupState_) {
            vector<Rank> comb = dynamic_cast<const StrategoDomain *>(getDomain())->startFigures_;
            do {
                if (id == actionID) return make_shared<StrategoSetupAction>(id++, comb);
            } while (next_permutation(comb.begin(), comb.end()));

            return make_shared<Action>();
        }
        for (int i = 0; i < boardState_.size(); i++)
        {
            if (!isPlayers(boardState_[i], player) || getRank(boardState_[i]) == 'B' || getRank(boardState_[i]) == 'F') continue;
            if (canMoveUp(i, boardState_, height, width)) {
                if (id == actionID) return make_shared<StrategoMoveAction>(id++, i, i - width, width);
                id++;
            }
            if (canMoveRight(i, boardState_, width)) {
                if (id == actionID) return make_shared<StrategoMoveAction>(id++, i, i + 1, width);
                id++;
            }
            if (canMoveLeft(i, boardState_, width)) {
                if (id == actionID) return make_shared<StrategoMoveAction>(id++, i, i - 1, width);
                id++;
            }
            if (canMoveDown(i, boardState_, height, width)) {
                if (id == actionID) return make_shared<StrategoMoveAction>(id++, i, i + width, width);
                id++;
            }
        }
        return make_shared<Action>();
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
        for (int i = 0; i < boardState_.size(); i++)
        {
            if (!isPlayers(boardState_[i], player) || getRank(boardState_[i]) == 'B' || getRank(boardState_[i]) == 'F') continue;
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
        if (isSetupState_) {
        return {0, 1};
        } else if (isFinished_){
            return {};
        } else{
            return {currentPlayer_};
        }
    }

    string StrategoState::toString() const {
        int w = dynamic_cast<const StrategoDomain *>(getDomain())->boardWidth_, h = dynamic_cast<const StrategoDomain *>(getDomain())->boardHeight_;
        string ret = "current player: " + to_string(currentPlayer_) + "\ncurrent state:";
        for (int i = 0; i < h; i++) {
            ret += "\n";
            for (int j = 0; j < w; j++)
            {
                CellState fig = boardState_[w*i + j];
                if (fig == ' ') ret += "__";
                else if (fig == 'L') ret += "LL";
                else
                {
                    ret += isPlayers(fig, 0) ? '0' : '1';
                    ret += getRank(fig);
                }
                ret += ' ';
            }
        }
        return ret;
    }

    OutcomeDistribution StrategoState::performSetupAction(const vector<shared_ptr<Action>> &actions, const StrategoDomain * stratDomain) const {
        OutcomeDistribution newOutcomes;
        StrategoSetupAction actionpl0 = dynamic_cast<StrategoSetupAction &>(*actions[0]); // player 0 setup
        StrategoSetupAction actionpl1 = dynamic_cast<StrategoSetupAction &>(*actions[1]); // player 1 setup
        vector<CellState> board = stratDomain->emptyBoard_;
        for (int i = 0; i < actionpl0.figuresSetup.size(); i++)
        {
            board[i] = getCellState(actionpl0.figuresSetup[i], 0);
            board[board.size() - 1 - i] = getCellState(actionpl1.figuresSetup[i], 1);
        }
        const auto newState = make_shared<StrategoState>(stratDomain, board, false, false, 0, 0);
        shared_ptr<StrategoObservation> obs = make_shared<StrategoObservation>();
        const auto newOutcome = Outcome(newState, {obs, obs}, obs, {0,0});
        newOutcomes.emplace_back(OutcomeEntry(newOutcome));
        return newOutcomes;
    }

    OutcomeDistribution StrategoState::performMoveAction(const vector<shared_ptr<Action>> &actions, const StrategoDomain * stratDomain) const {
        OutcomeDistribution newOutcomes;
        StrategoMoveAction action = dynamic_cast<StrategoMoveAction &>(*actions[currentPlayer_]);
        vector<CellState> board = boardState_;
        bool pl0won = false, pl1won = false;
        if (getRank(boardState_[action.endPos]) == 'F')
        {
            board[action.startPos] = ' ';
            board[action.endPos] = boardState_[action.startPos];
            if (currentPlayer_ == 0) pl0won = true;
            else pl1won = true;
        }
        else if (getRank(boardState_[action.startPos]) == getRank(boardState_[action.endPos]))
        {
            board[action.startPos] = ' ';
            board[action.endPos] = ' ';
        }
        else if (isFigureSlain(boardState_[action.startPos], boardState_[action.endPos]))
        {
            board[action.startPos] = ' ';
            board[action.endPos] = boardState_[action.startPos];
        }
        else
        {
            board[action.startPos] = ' ';
        }

        bool pl0f = false, pl1f = false;
        CellState pl0fig = ' ', pl1fig = ' ';
        int pl0MovableCounter = 0, pl1MovableCounter = 0;
        for (CellState f : board) {
            if (getRank(f) == 'B' || getRank(f) == 'F') continue;
            if (isPlayers(f, 0)) {
                pl0MovableCounter++;
                if (pl0MovableCounter == 1) pl0fig = getRank(f);
                pl0f = true;
            }
            if (isPlayers(f, 1)) {
                pl1MovableCounter++;
                if (pl1MovableCounter == 1) pl1fig = getRank(f);
                pl1f = true;
            }
        }

        if (!pl1f) pl0won = true;
        if (!pl0f) pl1won = true;
        if ((pl0MovableCounter == 1) && (pl1MovableCounter == 1) && (pl0fig == pl1fig))
        {
            pl0won = true;
            pl1won = true;
        }
        const vector<double> newRewards = { (pl0won ? 1.0 : 0.0) + (pl1won ? (-1.0) : 0.0), (pl1won ? 1.0 : 0.0) + (pl0won ? (-1.0) : 0.0)};
        shared_ptr<StrategoObservation> obs = make_shared<StrategoObservation>(action.startPos, action.endPos,
                                                                               boardState_[action.endPos] == ' ' ? 0 : boardState_[action.startPos],
                                                                               boardState_[action.endPos] == ' ' ? 0 : boardState_[action.endPos]);
        if (boardState_[action.endPos] == ' ' && (noAttackCounter_ == maxMovesWithoutAttack(stratDomain->boardWidth_, stratDomain->boardHeight_)))
        {
            const auto newState = make_shared<StrategoState>(stratDomain, board, false, true,
                                                             currentPlayer_ == 0 ? 1 : 0,
                                                             0);
            const auto newOutcome = Outcome(newState, {obs, obs}, obs, newRewards);
            newOutcomes.emplace_back(OutcomeEntry(newOutcome, 1.0));
        }
        else {

            const auto newState = make_shared<StrategoState>(stratDomain, board, false, pl0won || pl1won,
                                                             currentPlayer_ == 0 ? 1 : 0,
                                                             (boardState_[action.endPos] == ' ') ?
                                                             noAttackCounter_ + 1 : 0);
            const auto newOutcome = Outcome(newState, {obs, obs}, obs, newRewards);
            newOutcomes.emplace_back(OutcomeEntry(newOutcome, 1.0));
        }
        return newOutcomes;
    }

    OutcomeDistribution StrategoState::performActions(const vector<shared_ptr<Action>> &actions) const {
        const auto stratdomain = dynamic_cast<const StrategoDomain *>(domain_);

        if (isSetupState_) return performSetupAction(actions, stratdomain);
        else return performMoveAction(actions, stratdomain);
    }

    bool StrategoState::isTerminal() const {
        return isFinished_;
    }

    bool StrategoState::operator==(const State &rhs) const {
        auto state = dynamic_cast<const StrategoState &>(rhs);

        return hash_ == state.hash_
               && currentPlayer_ == state.currentPlayer_
               && isSetupState_ == state.isSetupState_
               && isFinished_ == state.isFinished_
               && boardState_ == state.boardState_;
    }
}