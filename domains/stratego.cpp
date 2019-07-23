//
// Created by Nikita Milyukov on 2019-06-26.
//

#include "domains/stratego.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2::domains {

    // rank < 96, startpos/endpos < 512
    unsigned int encodeAction(int startPos, int endPos, unsigned char startRank, unsigned char endRank)
    {
        return (startPos << 21) | (endPos << 12) | ((startRank - 32) << 6) | (endRank - 32); // 30 bits total
    }

    unsigned int encodeSetup(int id1, int id2)
    {
        return (3 << 30) | id1 | id2;
    }

    int maxNoAction(int h, int w)
    {
        return (2*h + 2*w - 4)*2;
    }

    bool isPlayers(unsigned char figure, int player)
    {
        if (figure == 'L')  return false;
        if  (figure == ' ') return false;
        return (((player == 0) && (figure < 128)) || ((player == 1) && (figure >= 128))) && (figure != 'L');
    }

    unsigned char makePlayer1(unsigned char figure)
    {
        return figure + 128;
    }

    bool isSamePlayer(int figure1, int figure2)
    {
        if (figure2 == ' ') return false;
        return ((figure1 > 128) && (figure2 > 128)) || ((figure1 < 128) && (figure2 < 128)) ;
    }

    unsigned char getRank(unsigned char figure)
    {
        return figure < 128 ? figure : figure - 128;
    }

    bool isBattleWon(unsigned char figure1, unsigned char figure2)
    {
        figure1 = getRank(figure1);
        figure2 = getRank(figure2);
        if (figure2 == ' ') return true;
        if (figure2 == 'B') return false;
        if (figure1 > figure2) return true;
        return false;
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

    vector<unsigned char> StrategoSettings::generateBoard() {
        vector<unsigned char> board(getBoardSize());
        std::fill(board.begin(), board.end(), ' ');
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

    StrategoObservation::StrategoObservation(const int startPos, const int endPos, const unsigned char startRank, const unsigned char endRank)
            : Observation(),
              startPos_(startPos),
              endPos_(endPos),
              startRank_(startRank),
              endRank_(endRank) {

        //assert(startPos+endPos > 0);

        id_ = encodeAction(startPos_, endPos_, startRank_, endRank_);
    }

    StrategoObservation::StrategoObservation(const int id1, int id2)
            : Observation(),
              startPos_(0),
              endPos_(0),
              startRank_(0),
              endRank_(0) {

        //assert(startPos+endPos > 0);

        id_ = encodeSetup(id1, id2);
    }
//maxNoAction(settings.boardHeight, settings.boardWidth)*settings.figures.size()
    StrategoDomain::StrategoDomain(StrategoSettings settings) :
            Domain(11,//maxNoAction(settings.boardHeight, settings.boardWidth)*settings.figures.size()*2,
                    2, true, make_shared<Action>(),
                   make_shared<StrategoObservation>()),
            startBoard_(settings.generateBoard()),
            startFigures_(settings.figures),
            boardWidth_(settings.boardWidth),
            boardHeight_(settings.boardHeight){
//        assert(startBoard_.size() > 1);
        const auto newState = make_shared<StrategoState>(this, startBoard_, true, false, 0, 0);
        shared_ptr<StrategoObservation> obs = make_shared<StrategoObservation>(0, 0, 0, 0);
        Outcome outcome(newState, {obs, obs}, obs, {0.0, 0.0});
        maxUtility_ = 1.0;
        rootStatesDistribution_.emplace_back(OutcomeEntry(outcome));
    }

    string StrategoDomain::getInfo() const {
        return "Stratego game with " + to_string(startBoard_.size()) + " cells";
    }

    int fact(int n)
    {
        if (n == 0) return 1;
        if (n == 1) return 1;
        return n*fact(n-1);
    }

    unsigned long StrategoState::countAvailableActionsFor(Player player) const {
        const auto stratdomain = dynamic_cast<const StrategoDomain *>(domain_);
        if (setupState_)
        {
            return fact(stratdomain->startFigures_.size());
        }
        else
        {
            int count = 0;
            for (int i = 0; i < boardState_.size(); i++) {
                if (isPlayers(boardState_[i], player)) {
                    if ((i+1) > stratdomain->boardWidth_) // i not in the top row
                        if (!isSamePlayer(boardState_[i], boardState_[i - stratdomain->boardWidth_]) && (boardState_[i - stratdomain->boardWidth_] != 'L'))
                            count++;
                    if ((i+1) % stratdomain->boardWidth_ != 0) // i not in the right column
                        if (!isSamePlayer(boardState_[i], boardState_[i + 1]) && (boardState_[i + 1] != 'L'))
                            count++;
                    if ((i+1) % stratdomain->boardWidth_ != 1) // i not in the left column
                        if (!isSamePlayer(boardState_[i], boardState_[i - 1]) && (boardState_[i - 1] != 'L'))
                            count++;
                    if (stratdomain->startBoard_.size() - (i+1) >= stratdomain->boardWidth_)// i not in the bottom row
                        if (!isSamePlayer(boardState_[i], boardState_[i + stratdomain->boardWidth_]) && (boardState_[i + stratdomain->boardWidth_] != 'L'))
                            count++;
                }
            }
            return count;
        }
    }

    void permute(vector<unsigned char> a, int l, int r, vector<shared_ptr<Action>> * actions, int * id)
    {
        // Base case
        if (l == r)
            actions->push_back(make_shared<StrategoSetupAction>((*id)++, a));
        else
        {
            // Permutations made
            for (int i = l; i <= r; i++)
            {

                // Swapping done
                std::swap(a[l], a[i]);

                // Recursion called
                permute(a, l+1, r, actions, id);

                //backtrack
                std::swap(a[l], a[i]);
            }
        }
    }

    vector<shared_ptr<Action>> StrategoState::getAvailableActionsFor(const Player player) const {
        vector<shared_ptr<Action>> actions;
        const auto stratdomain = dynamic_cast<const StrategoDomain *>(domain_);
        int id = 0;
        if (setupState_) {
            vector<unsigned char> comb = dynamic_cast<const StrategoDomain *>(getDomain())->startFigures_;
            permute(comb, 0, comb.size()-1, &actions, &id);
        } else
        {
            for (int i = 0; i < boardState_.size(); i++)
            {
                if (isPlayers(boardState_[i], player))
                {
                    if (getRank(boardState_[i]) == 'B' || getRank(boardState_[i]) == 'F') continue;
                    if ((stratdomain->boardHeight_ > 1) && ((i+1) > stratdomain->boardWidth_)) // i not in the top row
                        if (!isSamePlayer(boardState_[i], boardState_[i - stratdomain->boardWidth_]) && (boardState_[i - stratdomain->boardWidth_] != 'L'))
                            actions.push_back(make_shared<StrategoMoveAction>(id++, i, i - stratdomain->boardWidth_, stratdomain->boardWidth_));
                    if ((stratdomain->boardWidth_ > 1) && ((i+1) % stratdomain->boardWidth_ != 0)) // i not in the right column
                        if (!isSamePlayer(boardState_[i], boardState_[i + 1]) && (boardState_[i + 1] != 'L'))
                            actions.push_back(make_shared<StrategoMoveAction>(id++, i, i + 1, stratdomain->boardWidth_));
                    if ((stratdomain->boardWidth_ > 1) && ((i+1) % stratdomain->boardWidth_ != 1)) // i not in the left column
                        if (!isSamePlayer(boardState_[i], boardState_[i - 1]) && (boardState_[i - 1] != 'L'))
                            actions.push_back(make_shared<StrategoMoveAction>(id++, i, i - 1, stratdomain->boardWidth_));
                    if ((stratdomain->boardHeight_ > 1) && (stratdomain->startBoard_.size() - (i+1) >= stratdomain->boardWidth_))// i not in the bottom row
                        if (!isSamePlayer(boardState_[i], boardState_[i + stratdomain->boardWidth_]) && (boardState_[i + stratdomain->boardWidth_] != 'L'))
                            actions.push_back(make_shared<StrategoMoveAction>(id++, i, i + stratdomain->boardWidth_, stratdomain->boardWidth_));
                }
            }
        }
        return actions;
    }

    vector<Player> StrategoState::getPlayers() const {
        if (setupState_) {
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
                unsigned char fig = boardState_[w*i + j];
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

    OutcomeDistribution StrategoState::performActions(const vector<shared_ptr<Action>> &actions) const {
        const auto stratdomain = dynamic_cast<const StrategoDomain *>(domain_);
        OutcomeDistribution newOutcomes;
        if (setupState_)
        {
            StrategoSetupAction actionpl0 = dynamic_cast<StrategoSetupAction &>(*actions[0]); // player 0 setup
            StrategoSetupAction actionpl1 = dynamic_cast<StrategoSetupAction &>(*actions[1]); // player 1 setup
            vector<unsigned char> board = (dynamic_cast<const StrategoDomain *>(getDomain())->startBoard_);
            for (int i = 0; i < actionpl0.figuresSetup.size(); i++)
            {
                board[i] = actionpl0.figuresSetup[i];
                board[board.size() - 1 - i] = makePlayer1(actionpl1.figuresSetup[i]);
            }
            const auto newState = make_shared<StrategoState>(stratdomain, board, false, false, 0, 0);
            shared_ptr<StrategoObservation> obs = make_shared<StrategoObservation>(actionpl0.getId(), actionpl1.getId());
            const auto newOutcome = Outcome(newState, {obs, obs}, obs, {0,0});
            newOutcomes.emplace_back(OutcomeEntry(newOutcome, 1.0));
        } else
        {
            StrategoMoveAction action = dynamic_cast<StrategoMoveAction &>(*actions[currentPlayer_]);
            vector<unsigned char> board = boardState_;
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
            else if (isBattleWon(boardState_[action.startPos], boardState_[action.endPos]))
            {
                board[action.startPos] = ' ';
                board[action.endPos] = boardState_[action.startPos];
            }
            else
            {
                board[action.startPos] = ' ';
            }

            bool pl0f = false, pl1f = false;
            unsigned char pl0fig = ' ', pl1fig = ' ';
            int pl0counter = 0, pl1counter = 0;
            for (unsigned char f : board) {
                if (isPlayers(f, 0)) {
                    pl0counter++;
                    if (pl0counter == 1) pl0fig = getRank(f);
                    pl0f = true;
                }
                if (isPlayers(f, 1)) {
                    pl1counter++;
                    if (pl1counter == 1) pl1fig = getRank(f);
                    pl1f = true;
                }
            }

            if (!pl1f) pl0won = true;
            if (!pl0f) pl1won = true;
            if ((pl0counter == 1) && (pl1counter == 1) && (pl0fig == pl1fig))
            {
                pl0won = true;
                pl1won = true;
            }
            const vector<double> newRewards = { (pl0won ? 1.0 : 0.0) + (pl1won ? (-1.0) : 0.0), (pl1won ? 1.0 : 0.0) + (pl0won ? (-1.0) : 0.0)};
            shared_ptr<StrategoObservation> obs = make_shared<StrategoObservation>(action.startPos, action.endPos,
                    boardState_[action.endPos] == ' ' ? 0 : boardState_[action.startPos],
                    boardState_[action.endPos] == ' ' ? 0 : boardState_[action.endPos]);
            if (boardState_[action.endPos] == ' ' && (noActionCounter_ == maxNoAction(stratdomain->boardWidth_, stratdomain->boardHeight_)))
            {
                const auto newState = make_shared<StrategoState>(stratdomain, board, false, true,
                                                                 currentPlayer_ == 0 ? 1 : 0,
                                                                 0);
                const auto newOutcome = Outcome(newState, {obs, obs}, obs, newRewards);
                newOutcomes.emplace_back(OutcomeEntry(newOutcome, 1.0));
            }
            else {

                const auto newState = make_shared<StrategoState>(stratdomain, board, false, pl0won || pl1won,
                                                                 currentPlayer_ == 0 ? 1 : 0,
                                                                 (boardState_[action.endPos] == ' ') ?
                                                                 noActionCounter_ + 1 : 0);
                const auto newOutcome = Outcome(newState, {obs, obs}, obs, newRewards);
                newOutcomes.emplace_back(OutcomeEntry(newOutcome, 1.0));
            }
        }
        return newOutcomes;
    }

    bool StrategoState::isTerminal() const {
        return isFinished_;
    }

    bool StrategoState::operator==(const State &rhs) const {
        auto state = dynamic_cast<const StrategoState &>(rhs);

        return hash_ == state.hash_
               && currentPlayer_ == state.currentPlayer_
               && setupState_ == state.setupState_
               && isFinished_ == state.isFinished_
               && boardState_ == state.boardState_;
    }
}