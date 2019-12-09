/**
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


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef DOMAINS_KRIEGSPIEL_H_
#define DOMAINS_KRIEGSPIEL_H_

#include "base/base.h"
#include "chessboard_factory.h"

namespace GTLib2::domains {
class KriegspielState;

namespace chess {
enum player { WHITE = 0, BLACK = 1 };
enum pieceName { PAWN = 'p', KNIGHT = 'n', BISHOP = 'b', ROOK = 'r', QUEEN = 'q', KING = 'k' };
struct Square {
    int x;
    int y;
    Square(int p1, int p2) : x(p1), y(p2) {}
    inline bool operator==(const Square &that) const {
        return this->x == that.x && this->y == that.y;
    }
    inline HashType getHash() const { return hashCombine(935453154564551, x, y); }
};

/**
 * Converts Square object to a string eg. Square(3, 4) -> "c4"
 *
 * @param Square
 * @returns std::string
 */
const string coordToString(Square c);

/**
 * Converts a string into a Square object eg. "c4" -> Square(3, 4)
 *
 * @param std::string
 * @returns Square
 */
const Square stringToCoord(string s);

/**
 * Inverts player color (returns chess::BLACK for chess::WHITE and vice versa.
 *
 * @param int
 * @returns int
 */
int invertColor(int);

/**
 * AbstractPiece class for Kriegspiel, represents an abstract figure on the board
 */
class AbstractPiece {
 public:

    /**
     * Constructor for an abstract piece
     * @param pieceName enum, the kind of figure (eg. chess::PAWN)
     * @param int color
     * @param Square position
     * @param const GTLib2::domains::KriegspielState* the board the piece is on
     */
    AbstractPiece(pieceName, int, Square, const GTLib2::domains::KriegspielState *s);
    inline ~AbstractPiece() {

    };
    /**
     * Returns all (valid and non-valid) moves of a figure
     */
    vector<Square> *getAllMoves() const;

    /**
     * Returns all valid moves of a figure
     */
    vector<Square> *getAllValidMoves() const;

    /**
     * Returns list of Squares this figure is currently attacking (serves for limiting king's movement
     * so that when a square is attacked by another figure, the king cannot move there
     */
    virtual vector<Square> getSquaresAttacked() const = 0;

    /**
     * Updates the figure's list of moves (non-valid)
     */
    virtual void updateMoves() = 0;

    /**
     * Updates the figure's list of moves (valid)
     * Also updates which figures this figure is protecting and which enemy figures it is pinning (if any)
     *
     * @param bool updates only pins and protections (not moves, used for when the figure itself is pinned but still can be
     * protecting and pinning other figures)
     */
    virtual void updateValidMovesPinsProtects(bool) = 0;

    /**
     * Updates valid moves while the figure is pinned (the line towards the pinner and the pinner itself)
     */
    void updateValidMovesWhilePinned();

    /**
     * Updates valid moves of the figure while the player is in check (the checker's position and blocking moves)
     */
    void updateValidMovesWhileInCheck();

    /**
     * Returns a string used for figure notation eg. K for king and N for knight
     */
    string toString() const;
    bool operator==(const AbstractPiece &that) const;
    bool equal_to(const AbstractPiece &) const;
    int getColor() const;

    /**
     * Performs an update of moves depending on the circumstances (pinned/not pinned, in check/not in check)
     */
    void update();
    inline pieceName getKind() const {
        return this->kind;
    }
    inline void setBoard(const GTLib2::domains::KriegspielState *s) {
        this->board = s;
    }

    /**
     * Updates the figure's position
     */
    void move(Square);
    bool hasMoved() const;
    AbstractPiece *getProtectedBy() const;
    AbstractPiece *getPinnedBy() const;
    void setProtectedBy(AbstractPiece *);
    void setPinnedBy(AbstractPiece *);
    Square getPosition() const;

    /**
     * Clears the figure's moves and pinned by/protected by variables
     */
    void reset();
    void setHasMoved(bool);
    inline const GTLib2::domains::KriegspielState *getBoard() const {
        return this->board;
    }
    virtual shared_ptr<AbstractPiece> clone() const = 0;
 protected:
    int color;
    shared_ptr<vector<Square>> moves = make_shared<vector<Square>>();
    shared_ptr<vector<Square>> validMoves = make_shared<vector<Square>>();
    pieceName kind;
    Square position;
    const GTLib2::domains::KriegspielState *board;
    bool moved = false;
    AbstractPiece *protectedBy = nullptr;
    AbstractPiece *pinnedBy = nullptr;
};

/**
 * Pawn class for Kriegspiel, represents a Pawn on the board
 * For documentation of functions, see superior class
 */
class Pawn: public AbstractPiece {
 public:
    /**
     * Constructor for the Pawn class
     * @param pieceName enum, the kind of figure (eg. chess::PAWN)
     * @param int color
     * @param Square position
     * @param const GTLib2::domains::KriegspielState* the board the piece is on
     * @param int id of the pawn, used for calculation of pawn's take moves for observations
     */
    Pawn(pieceName, int, Square, const GTLib2::domains::KriegspielState *, int);
    void updateMoves() override;
    vector<Square> getSquaresAttacked() const override;
    void updateValidMovesPinsProtects(bool) override;
    shared_ptr<AbstractPiece> clone() const override;
    int getId() const;
 protected:
    int id;
};

/**
 * Bishop class for Kriegspiel, represents a Bishop on the board
 * For documentation of functions, see superior class
 */
class Bishop: public AbstractPiece {
 public:
    Bishop(pieceName, int, Square, const GTLib2::domains::KriegspielState *);
    void updateMoves() override;
    vector<Square> getSquaresAttacked() const override;
    void updateValidMovesPinsProtects(bool) override;
    shared_ptr<AbstractPiece> clone() const override;
};

/**
 * Rook class for Kriegspiel, represents a Rook on the board
 * For documentation of functions, see superior class
 */
class Rook: public AbstractPiece {
 public:
    Rook(pieceName, int, Square, const GTLib2::domains::KriegspielState *);
    Rook(pieceName, int, Square, const GTLib2::domains::KriegspielState *, bool);
    void updateMoves() override;
    vector<Square> getSquaresAttacked() const override;
    void updateValidMovesPinsProtects(bool) override;
    shared_ptr<AbstractPiece> clone() const override;
};

/**
 * Knight class for Kriegspiel, represents a Knight on the board
 * For documentation of functions, see superior class
 */
class Knight: public AbstractPiece {
 public:
    Knight(pieceName, int, Square, const GTLib2::domains::KriegspielState *);
    void updateMoves() override;
    vector<Square> getSquaresAttacked() const override;
    void updateValidMovesPinsProtects(bool) override;
    shared_ptr<AbstractPiece> clone() const override;
};

/**
 * Queen class for Kriegspiel, represents a Queen on the board
 * For documentation of functions, see superior class
 */
class Queen: public AbstractPiece {
 public:
    Queen(pieceName, int, Square, const GTLib2::domains::KriegspielState *);
    void updateMoves() override;
    vector<Square> getSquaresAttacked() const override;
    void updateValidMovesPinsProtects(bool) override;
    shared_ptr<AbstractPiece> clone() const override;
};

/**
 * King class for Kriegspiel, represents a King on the board
 * For documentation of functions, see superior class
 */
class King: public AbstractPiece {
 public:
    King(pieceName, int, Square, const GTLib2::domains::KriegspielState *);
    King(pieceName, int, Square, const GTLib2::domains::KriegspielState *, bool);
    void updateMoves() override;
    vector<Square> getSquaresAttacked() const override;
    void updateValidMovesPinsProtects(bool) override;
    shared_ptr<AbstractPiece> clone() const override;
};

// See https://en.wikipedia.org/wiki/Minichess for variants description
enum BOARD: int { STANDARD, SILVERMAN4BY4, MINIMAL3x3, MICROCHESS, DEMICHESS };
class BoardFactory;
struct boardInfo;
}

using chess::AbstractPiece;
using chess::coordToString;

/**
 * KriegspielAction class represents a single action in the game of Kriegspiel
 */
class KriegspielAction: public Action {
 public:
    /**
     * KriegspielAction constructor
     * @param int id
     * @param pair<shared_ptr<AbstractPiece>, chess::Square>, pair where the first member is the piece being moved and second is
     *                                                        the position to which it is moving
     * @param chess::Square, the square where the figure is moving from
     */
    KriegspielAction(ActionId id, pair<shared_ptr<AbstractPiece>, chess::Square>, chess::Square);
    explicit KriegspielAction(ActionId id);
    inline KriegspielAction() : KriegspielAction(NO_ACTION) {}
    inline string toString() const final {
        if (id_ == NO_ACTION)
            return "No action";
        return move_.first->toString() + coordToString(move_.second);
    }
    bool operator==(const Action &that) const override;
    pair<shared_ptr<AbstractPiece>, chess::Square> getMove() const;
    chess::Square movingFrom() const;
    HashType getHash() const override;
    shared_ptr<KriegspielAction> clone() const;
 private:
    const pair<shared_ptr<AbstractPiece>, chess::Square> move_;
    chess::Square moveFrom;
};

/**
 * KriegspielDomain class represents the domain of the game of Kriegspiel
 */
class KriegspielDomain: public Domain {
 public:
    // constructor
    /**
     * KriegspielDomain constructor
     * @param int maxDepth, maximum depth when counting both legal and illegal half-moves
     * @param int legalMaxDepth, maximum depth when counting only legal half-moves
     * @param chess::BOARD enum, type of chess board for initialization
     */
    explicit KriegspielDomain(unsigned int maxDepth, unsigned int legalMaxDepth, chess::BOARD);

    /**
     * KriegspielDomain constructor
     * @param int maxDepth, maximum depth when counting both legal and illegal half-moves
     * @param int legalMaxDepth, maximum depth when counting only legal half-moves
     * @param string, FEN string from which to create the board
     */
    explicit KriegspielDomain(unsigned int maxDepth, unsigned int legalMaxDepth, string);

    // destructor
    ~KriegspielDomain() override = default;

    // GetInfo returns string containing domain information.
    string getInfo() const final;
};

/**
 * KriegspielObservation class represents the observation for the game of Kriegspiel
 */
class KriegspielObservation: public Observation {
 public:
    inline KriegspielObservation() : Observation() {}
    explicit KriegspielObservation(int id);

    inline string toString() const final {
        if (id_ == 1)
            return "Success";
        return "Fail";
    };
};

/**
 * KriegspielState class represents a board state in the game of Kriegspiel
 */
class KriegspielState: public State {
 public:

    // Constructor
    /**
     * @param Domain the Kriegspiel domain
     * @param int legalMaxDepth, the depth of game only when counting legal half-moves
     * @param chess::BOARD the board type of the game
     */
    KriegspielState(const Domain *domain, int, chess::BOARD);

    /**
     * @param Domain the Kriegspiel domain
     * @param int legalMaxDepth, the depth of game only when counting legal half-moves
     * @param stringthe string from which the board is constructed in FEN notation
     */
    KriegspielState(const Domain *domain, int, string);

    /**
     * @param Domain the Kriegspiel domain
     * @param int legalMaxDepth, the depth of game only when counting legal half-moves
     * @param int x-size of the board
     * @param int y-size of the board
     * @param shared_ptr<vector<shared_ptr<AbstractPiece>>> a list containing all of the pieces on the board
     * @param Square enPassantSquare, the square where an en passant capture is possible (if any, otherwise null)
     * @param shared_ptr<vector<shared_ptr<KriegspielAction>>>, the current move history of the board
     * @param int p, the player that is on the move (chess::WHITE or chess::BLACK)
     * @param bool castle, whether castling is possible on the current board
     * @param shared_ptr<vector<shared_ptr<KriegspielAction>>> attemptedMoves, a history of attempted moves (non-legal moves)
     */
    KriegspielState(const Domain *domain,
                    int,
                    int,
                    int,
                    shared_ptr<vector<shared_ptr<AbstractPiece>>>  pieces,
                    chess::Square enPassantSquare,
                    const shared_ptr<vector<shared_ptr<KriegspielAction>>>&,
                    int,
                    bool,
                    shared_ptr<vector<shared_ptr<KriegspielAction>>> );

    // Destructor
    ~KriegspielState() override = default;

    unsigned long countAvailableActionsFor(Player player) const override;

    // GetActions returns possible actions for a player in the state.
    vector<shared_ptr<Action>> getAvailableActionsFor(Player player) const override;

    /**
     * Performs an action on the current board state
     * @returns OutcomeDistribution containing the Outcome(a new state (should be a completely new object), observations for the players, rewards for the players)
     *                              and the NaturalProbability of the Outcome
     */
    OutcomeDistribution  performActions(const vector <shared_ptr<Action>> &actions) const override;

    /**
     * Gets the player(s) moving in the current game state
     * @returns vector<Player> a list of the players
     */
    inline vector<Player> getPlayers() const final {
        vector<Player> v;
        if (!this->gameHasEnded || this->moveHistory->size() == domain_->getMaxStateDepth())
            v.emplace_back(playerOnTheMove);
        return v;
    }

    inline bool isTerminal() const override { return gameHasEnded; };

    bool operator==(const State &rhs) const override;

    void updateAllPieces() const;

    /**
     * Updates all pieces of a color
     * @param int color of the pieces to be updated
     */
    void updatePiecesOfColor(int color) const;

    /**
     * Fetches all of the pieces of a certain color
     * @param int color of the pieces to be fetched
     * @returns vector<shared_ptr<AbstractPiece>> a list of the pieces
     */
    vector<shared_ptr<AbstractPiece>> getPiecesOfColor(int) const;

    // for testing
    void clearBoard();
    void insertPiece(shared_ptr<AbstractPiece>);
    void setPlayerOnMove(int);

    string toString() const override;

    /**
     * Function for calculating the rewards for the current game state
     * @returns vector<double> a list of rewards for all of the players
     */
    vector<double> checkGameOver() const;

    /**
     * Checks whether an action produces an available enPassantSquare
     * @returns chess::Square the square where the en passant is possible (if any) or null
     */
    chess::Square checkEnPassant(KriegspielAction *) const;

    /**
     * Checks whether a piece is pinned by another piece
     * @param AbstractPiece* the pinner
     * @param AbstractPiece* the pinned
     * @returns bool whether the pinner is pinning the pinned
     */
    bool checkPinnedPiece(AbstractPiece *, AbstractPiece *) const;

    /**
     * Gets all of the pieces of a certain color and kind
     * @param int color
     * @param chess::pieceName enum the kind of piece
     * @returns vector<shared_ptr<AbstractPiece>> a list of the colors that fit the parameters
     */
    vector<shared_ptr<AbstractPiece>> getPiecesOfColorAndKind(int, chess::pieceName) const;

    /**
     * Fetches a piece on certain coordinates
     * @param chess::Square the square for which to fetch the figure
     * @returns shared_ptr<AbstractPiece> the piece on the square in question (if any) or null
     */
    shared_ptr<AbstractPiece> getPieceOnCoords(chess::Square) const;

    /**
     * Returns a list of squares beyond a certain figure that is being pierced by another figure
     * @param AbstractPiece* the piercing figure
     * @param AbstractPiece* the pierced figure
     * @returns vector<chess::Square> a list of the squares beyond the pierced figure
     */
    vector<chess::Square> getSquaresPierced(AbstractPiece *, AbstractPiece *) const;

    /**
     * Returns a list of squares between two figures
     * @param AbstractPiece* the first figure
     * @param AbstractPiece* the second figure
     * @returns vector<chess::Square> a list of squares between the two figures
     */
    vector<chess::Square> getSquaresBetween(AbstractPiece *, AbstractPiece *) const;

    /**
     * Returns a list of figures that are checking the king of the player currently on the move (if any)
     * @returns vector<shared<ptr<AbstractPiece>> the checking figures
     */
    vector<shared_ptr<AbstractPiece>> getCheckingFigures() const;

    chess::Square getEnPassantSquare() const;

    /**
     * Performs a castling action
     * @param KriegspielAction* an action that has previously been deemed as a castle
     */
    void castle(KriegspielAction *) const;

    /**
     * Checks whether the current Square is beyond this board
     * @param chess::Square the square to check for
     * @returns bool whether or not the Square is outside of the bounds of this board
     */
    bool coordOutOfBounds(chess::Square) const;

    /**
     * Returns this->playerInCheck
     */
    int isPlayerInCheck() const;

    /**
     * Checks whether the player currently on the move is in check
     * The result is stored in this->playerInCheck (either chess::WHITE/chess::BLACK if a player is in check, -1 otherwise)
     */
    void checkPlayerInCheck();

    /**
     * Same principle as KriegspielState::checkPlayerInCheck() but usable where the function required has to be marked as const
     * @param int color of the player who we are checking for
     * @returns the color of the player currently on the move if he is in check, -1 otherwise
     */
    int checkGameOverCheck(int) const;

    /**
     * Resets all of the pieces (by calling piece->reset(), see doc of reset in AbstractPiece::reset())
     */
    void resetAllPieces();

    /**
     * Attempts to perform a move in the game
     * @param KriegspielAction* the move to perform
     * @returns bool true if the move to be performed is valid, false otherwise
     */
    bool makeMove(KriegspielAction *a);

    /**
     * Updates the state for a player
     * @param int the color of player for who to perform the update
     */
    void updateState(int);

    /**
     * @returns bool whether or not castling is possible on this board
     */
    bool canCastle() const;

    /**
     * Checks whether a square is under attack by any piece from a certain player
     * @param int the color of the player
     * @param chess::Square the square to check for
     * @returns bool
     */
    bool isSquareUnderAttack(int, chess::Square) const;

    /**
     * Promotes a pawn if it reached the end of the board and puts a queen at his coordinates
     * @param shared_ptr<AbstractPiece> the pawn object (for deletion)
     * @param chess::Square the position where the queen should be put
     */
    void promote(shared_ptr<AbstractPiece>, chess::Square) const;
    int getYSize() const;
    int getXSize() const;

    /**
     * Calculates an observation for a player, takes into account:
     * -pieces lost in the last half-move
     * -pawn captures available at the last half-move
     *
     * @param int the player for which to calculate
     * @returns int the observation
     */
    int calculateObservation(Player player) const;
    void setGameHasEnded(bool gameHasEnded);
    void addToHistory(shared_ptr<KriegspielAction>);
    void addToAttemptedMoves(shared_ptr<KriegspielAction>);
    void setEnPassant(chess::Square);
    shared_ptr<vector<shared_ptr<AbstractPiece>>> copyPieces() const;
    shared_ptr<vector<shared_ptr<KriegspielAction>>> copyMoveHistory() const;
    shared_ptr<vector<shared_ptr<KriegspielAction>>> copyAttemptedMoves() const;
 protected:
    shared_ptr<vector<shared_ptr<AbstractPiece>>> pieces;  // players' board
    vector<shared_ptr<AbstractPiece>> checkingFigures;
    const shared_ptr<vector<shared_ptr<KriegspielAction>>> moveHistory;
    const shared_ptr<vector<shared_ptr<KriegspielAction>>> attemptedMoveHistory;
    Player playerOnTheMove;
    int lastCut = 0;
    chess::Square enPassantSquare;
    Player playerInCheck = -1;
    const int legalMaxDepth;
    bool gameHasEnded = false;
 private:
    void initBoard(chess::BOARD);
    void initBoard(string);
    int xSize;
    int ySize;
    bool canPlayerCastle;
};

}

#endif  // DOMAINS_KRIEGSPIEL_H_

#pragma clang diagnostic pop
