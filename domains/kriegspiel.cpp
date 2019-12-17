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


#include "domains/kriegspiel.h"

namespace GTLib2::domains::chess {

char coords[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
int bishopMoves[4][2] = {{-1, 1}, {-1, -1}, {1, -1}, {1, 1}};
int rookMoves[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
int queenKingMoves[8][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}, {-1, 1}, {-1, -1}, {1, -1}, {1, 1}};
int knightMoves[8][2] = {{1, 2}, {-1, 2}, {-1, -2}, {1, -2}, {2, 1}, {-2, 1}, {-2, -1}, {2, -1}};

const string coordToString(const Square c) {
    return coords[c.x - 1] + to_string(c.y);
}

const Square stringToCoord(string s) {
    int x = s[0] - 96;
    int y = s[1] - 48;
    return Square(x, y);
}


AbstractPiece::AbstractPiece(chess::pieceName k,
                             int c,
                             Square pos,
                             const KriegspielState *s)
    : color(c), kind(k), position(pos), board(s) {}

Pawn::Pawn(chess::pieceName k, int c, chess::Square p, const KriegspielState *b, int _id)
    : AbstractPiece(k, c, p, b), id(_id) {}
Rook::Rook(chess::pieceName k, int c, chess::Square p, const KriegspielState *b)
    : AbstractPiece(k, c, p, b) {}
Queen::Queen(chess::pieceName k, int c, chess::Square p, const KriegspielState *b)
    : AbstractPiece(k, c, p, b) {}
Knight::Knight(chess::pieceName k, int c, chess::Square p, const KriegspielState *b)
    : AbstractPiece(k, c, p, b) {}
Bishop::Bishop(chess::pieceName k, int c, chess::Square p, const KriegspielState *b)
    : AbstractPiece(k, c, p, b) {}
King::King(chess::pieceName k, int c, chess::Square p, const KriegspielState *b) :
    AbstractPiece(k, c, p, b) {}
King::King(chess::pieceName k, int c, chess::Square p, const KriegspielState *b, bool mo)
    : AbstractPiece(k, c, p, b) {
    this->moved = mo;
}
Rook::Rook(chess::pieceName k, int c, chess::Square p, const KriegspielState *b, bool mo)
    : AbstractPiece(k, c, p, b) {
    this->moved = mo;
}

shared_ptr<AbstractPiece> Queen::clone() const {
    return make_shared<Queen>(this->kind, this->color, this->position, this->board);
}

shared_ptr<AbstractPiece> Pawn::clone() const {
    return make_shared<Pawn>(this->kind, this->color, this->position, this->board, this->id);
}

shared_ptr<AbstractPiece> Knight::clone() const {
    return make_shared<Knight>(this->kind, this->color, this->position, this->board);
}

shared_ptr<AbstractPiece> King::clone() const {
    return make_shared<King>(this->kind, this->color, this->position, this->board, this->moved);
}

shared_ptr<AbstractPiece> Rook::clone() const {
    return make_shared<Rook>(this->kind, this->color, this->position, this->board, this->moved);
}

shared_ptr<AbstractPiece> Bishop::clone() const {
    return make_shared<Bishop>(this->kind, this->color, this->position, this->board);
}

int invertColor(int c) {
    return c == WHITE ? BLACK : WHITE;
}

void AbstractPiece::move(Square pos) {
    this->position = pos;
    this->moved = true;
}

bool AbstractPiece::hasMoved() const {
    if (this->kind != PAWN) {
        return this->moved;
    } else {
        if (this->getColor() == WHITE) {
            return this->position.y != 2;
        } else {
            return this->position.y != this->board->getYSize() - 1;
        }
    }
}

AbstractPiece *AbstractPiece::getPinnedBy() const {
    return this->pinnedBy;
}

AbstractPiece *AbstractPiece::getProtectedBy() const {
    return this->protectedBy;
}

void AbstractPiece::setPinnedBy(AbstractPiece *p) {
    this->pinnedBy = p;
}

void AbstractPiece::setProtectedBy(AbstractPiece *p) {
    this->protectedBy = p;
}

vector<Square> *AbstractPiece::getAllMoves() const {
    return this->moves.get();
}

vector<Square> *AbstractPiece::getAllValidMoves() const {
    return this->validMoves.get();
}

void AbstractPiece::update() {
    this->updateMoves();
    if (this->board->getPlayerInCheck() == this->getColor()) {
        if (this->getPinnedBy() != nullptr) {
            //pinned piece cant block check
            this->validMoves->clear();
            return;
        }
        this->updateValidMovesWhileInCheck();
    } else this->updateValidMovesPinsProtects(this->getPinnedBy() != nullptr);
}

void AbstractPiece::updateValidMovesWhileInCheck() {
    if (this->kind == KING) {
        for (int *i: queenKingMoves) {
            Square newPos(this->position.x + i[0], this->position.y + i[1]);
            if (this->board->coordOutOfBounds(newPos)) continue;
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if (this->board->isSquareUnderAttack(invertColor(this->color), newPos)
                || (p != nullptr && p->getColor() == this->getColor())
                || (p != nullptr && p->getProtectedBy() != nullptr)) {
                continue;
            }
            this->validMoves->push_back(newPos);
        }
        return;
    }
    if (this->board->getCheckingFigures().size() == 2) { //no chance to block 2 checks
        this->validMoves->clear();
        return;
    }
    auto king = this->board->getPiecesOfColorAndKind(this->getColor(), KING)[0];
    auto checker = this->board->getCheckingFigures().at(0);
    if (checker->getKind() != KNIGHT) {
        vector<Square> blockingSquares = this->board->getSquaresBetween(king.get(), checker.get());
        for (Square s: blockingSquares) {
            if (std::find(this->moves->begin(), this->moves->end(), s) != this->moves->end()) {
                this->validMoves->push_back(s);
            }
        }
    }
    if (std::find(this->moves->begin(), this->moves->end(), checker->getPosition())
        != this->moves->end()) {
        this->validMoves->push_back(checker->getPosition());
    }
}

void AbstractPiece::reset() {
    this->pinnedBy = nullptr;
    this->protectedBy = nullptr;
    this->moves->clear();
    this->validMoves->clear();
}

string AbstractPiece::toString() const {
    return std::string(1, this->color == chess::WHITE ? toupper(this->kind) : tolower(this->kind));
}

bool AbstractPiece::operator==(const chess::AbstractPiece &that) const {
    return this->getPosition() == that.getPosition() && this->getKind() == that.getKind()
        && this->getColor() == that.getColor();
}

int AbstractPiece::getColor() const {
    return this->color;
}

Square AbstractPiece::getPosition() const {
    return this->position;
}

void AbstractPiece::setHasMoved(bool h) {
    this->moved = h;
}

bool AbstractPiece::equal_to(const AbstractPiece &that) const {
    return this->getColor() == that.getColor() && this->getPosition() == that.getPosition()
        && this->getKind() == that.getKind();
}

void Pawn::updateMoves() {
    //cut moves
    int dy = this->color == WHITE ? 1 : -1;
    Square takeLeft(this->position.x - 1, this->position.y + dy);
    Square takeRight(this->position.x + 1, this->position.y + dy);
    Square moveForward(this->position.x, this->position.y + dy);
    Square moveByTwo = this->hasMoved() ? Square(-1, -1) :
                       Square(this->position.x, this->position.y + 2 * dy);

    for (const Square &move : {takeLeft, takeRight, moveForward}) {
        if (!this->board->coordOutOfBounds(move)) {
            auto piece = this->board->getPieceOnCoords(move);
            if (piece == nullptr || piece->getColor() != this->getColor()) {
                moves->push_back(move);
            }
        }
    }
    //add moveByTwo only if move by one is possible
    if (!this->board->coordOutOfBounds(moveByTwo)
        && (std::find(this->moves->begin(), this->moves->end(), moveForward)
            != this->moves->end())) {
        auto piece = this->board->getPieceOnCoords(moveByTwo);
        if (piece == nullptr || piece->getColor() != this->getColor()) {
            moves->push_back(moveByTwo);
        }
    }
}

void Pawn::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
    //cut moves
    int dy = this->color == WHITE ? 1 : -1;
    for (int i : {-1, 1}) {
        Square cutpos(this->position.x + i, this->position.y + dy);
        if (this->board->coordOutOfBounds(cutpos)) continue;
        shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(cutpos);

        if (p != nullptr && p->getColor() != this->getColor()) {
            if (!onlyPinsAndProtects) validMoves->push_back(cutpos);
        } else if (p != nullptr && p->getColor() == this->getColor()) {
            p->setProtectedBy(this);
        } else if (this->board->getEnPassantSquare() == cutpos) {
            if (!onlyPinsAndProtects) validMoves->push_back(cutpos);
        }
    }

    Square moveByOne(this->position.x, this->position.y + dy);
    Square moveByTwo(this->position.x, this->position.y + 2 * dy);
    if (!onlyPinsAndProtects) {
        if (this->board->getPieceOnCoords(moveByOne) == nullptr) {
            validMoves->push_back(moveByOne);
            if (!this->moved && this->board->getPieceOnCoords(moveByTwo) == nullptr) {
                validMoves->push_back(moveByTwo);
            }
        }
    }
}
void Pawn::updateValidMovesWhileInCheck() {
    if (this->board->getCheckingFigures().size() == 2) { //no chance to block 2 checks
        this->validMoves->clear();
        return;
    }
    int dy = this->color == WHITE ? 1 : -1;
    Square moveByOne(this->position.x, this->position.y + dy);
    Square moveByTwo = this->hasMoved() ? Square(-1, -1) :
                       Square(this->position.x, this->position.y + 2 * dy);

    auto king = this->board->getPiecesOfColorAndKind(this->getColor(), KING)[0];
    auto checker = this->board->getCheckingFigures().at(0);

    if (checker->getKind() != KNIGHT && this->board->getPieceOnCoords(moveByOne) == nullptr) {
        vector<Square> blockingSquares = this->board->getSquaresBetween(king.get(), checker.get());
        if (std::find(blockingSquares.begin(), blockingSquares.end(), moveByOne)
            != blockingSquares.end()) {
            this->validMoves->push_back(moveByOne);
        } else if (!this->hasMoved()
            && (std::find(blockingSquares.begin(), blockingSquares.end(), moveByTwo)
                != blockingSquares.end())) {
            this->validMoves->push_back(moveByTwo);
        }
    }
    auto attackedSquares = this->getSquaresAttacked();
    if (std::find(attackedSquares.begin(), attackedSquares.end(), checker->getPosition())
        != attackedSquares.end()) {
        this->validMoves->push_back(checker->getPosition());
    }
}

vector<Square> Pawn::getSquaresAttacked() const {
    vector<Square> attacked;
    int dy = this->color == WHITE ? 1 : -1;
    vector<Square> positions{
        Square(this->position.x - 1, this->position.y + dy),
        Square(this->position.x + 1, this->position.y + dy)};

    for (auto pos : positions) {
        if (!this->board->coordOutOfBounds(pos)) {
            attacked.push_back(pos);
        }
    }
    return attacked;
}

int Pawn::getId() const {
    return this->id;
}
void Pawn::updateValidMovesWhilePinned() {
    this->validMoves->clear();
    int dy = this->color == WHITE ? 1 : -1;
    Square attackLeft(this->position.x - 1, this->position.y + dy);
    Square attackRight(this->position.x + 1, this->position.y + dy);

    Square pinnerPosition = this->getPinnedBy()->getPosition();

    if ((pinnerPosition == attackLeft) || (pinnerPosition == attackRight)) {
        this->validMoves->push_back(pinnerPosition);
    }
    //same column
    if (pinnerPosition.x == this->position.x) {
        auto moveByOne = Square(this->position.x, this->position.y + dy);
        if (this->board->getPieceOnCoords(moveByOne) == nullptr) {
            this->validMoves->push_back(moveByOne);
            if (!this->hasMoved()) {
                auto moveByTwo = Square(this->position.x, this->position.y + 2 * dy);
                if (this->board->getPieceOnCoords(moveByTwo) == nullptr) {
                    this->validMoves->push_back(moveByTwo);
                }
            }
        }
    }
}

void Rook::updateMoves() {
    for (auto move: rookMoves) {
        int dx = this->position.x;
        int dy = this->position.y;
        bool breakInner = false;
        dx += move[0];
        dy += move[1];
        //for cycle runs until coordinates are out of bounds
        for (; !this->board->coordOutOfBounds(Square(dx, dy)); dx += move[0], dy += move[1]) {
            if (breakInner) continue;
            Square newPos(dx, dy);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if (p != nullptr && p->getColor() == this->getColor()) {
                breakInner = true;
                continue;
            }
            this->moves->push_back(newPos);
        }
    }
}

void Rook::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
    for (auto move: rookMoves) {
        int dx = this->position.x;
        int dy = this->position.y;
        bool breakInner = false;
        dx += move[0];
        dy += move[1];
        //for cycle runs until coordinates are out of bounds
        for (; !this->board->coordOutOfBounds(Square(dx, dy)); dx += move[0], dy += move[1]) {
            if (breakInner) break;
            Square newPos(dx, dy);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if (p != nullptr) {
                breakInner = true;
                if (p->getColor() == this->getColor()) {
                    p->setProtectedBy(this);
                    continue;
                } else {
                    if (this->board->checkPinnedPiece(this, p.get())) {
                        p->setPinnedBy(this);
                        p->updateValidMovesWhilePinned();
                    }
                }
            }
            if (!onlyPinsAndProtects) this->validMoves->push_back(newPos);
        }
    }
}

vector<Square> Rook::getSquaresAttacked() const {
    vector<Square> attacked;
    for (auto move: rookMoves) {
        int dx = this->position.x;
        int dy = this->position.y;
        dx += move[0];
        dy += move[1];
        //for cycle runs until coordinates are out of bounds
        for (; !this->board->coordOutOfBounds(Square(dx, dy)); dx += move[0], dy += move[1]) {
            Square newPos(dx, dy);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            attacked.push_back(newPos);
            // it is necessary to add squares behind enemy king so it knows it is still in check
            // if it moves one square away from the checker
            if ((p != nullptr && p->getKind() != KING)
                || (p != nullptr && p->getKind() == KING && p->getColor() == this->getColor())) {
                break;
            }
        }
    }
    return attacked;
}
void Rook::updateValidMovesWhilePinned() {
    AbstractPiece *pinner = this->getPinnedBy();
    this->validMoves->clear();

    int dx = this->position.x;
    int dy = this->position.y;

    Square pinnerPosition = pinner->getPosition();
    //same row
    if (dy == pinnerPosition.y) {
        int incrementX = dx < pinnerPosition.x ? 1 : -1;
        dx += incrementX;
        for (; dx != pinnerPosition.x; dx += incrementX) {
            this->validMoves->push_back(Square(dx, dy));
        }
        this->validMoves->push_back(pinnerPosition);

    } else if (dx == pinnerPosition.x) { // same column
        int incrementY = dy < pinnerPosition.y ? 1 : -1;
        dy += incrementY;
        for (; dy != pinnerPosition.y; dy += incrementY) {
            this->validMoves->push_back(Square(dx, dy));
        }
        this->validMoves->push_back(pinnerPosition);
    }
}

void Knight::updateMoves() {
    for (auto move: knightMoves) {
        Square newPos(this->position.x + move[0], this->position.y + move[1]);
        if (!this->board->coordOutOfBounds(newPos)) {
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if (p == nullptr || (p != nullptr && p->getColor() != this->getColor())) {
                this->moves->push_back(newPos);
            }
        }
    }
}

void Knight::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
    for (auto move: knightMoves) {
        Square newPos(this->position.x + move[0], this->position.y + move[1]);
        if (!this->board->coordOutOfBounds(newPos)) {
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if (p != nullptr && p->getColor() == this->getColor()) {
                p->setProtectedBy(this);
                continue;
            }
            if (!onlyPinsAndProtects) this->validMoves->push_back(newPos);
        }
    }
}

vector<Square> Knight::getSquaresAttacked() const {
    vector<Square> attacked;
    for (auto move: knightMoves) {
        Square newPos(this->position.x + move[0], this->position.y + move[1]);
        if (!this->board->coordOutOfBounds(newPos)) {
            attacked.push_back(newPos);
        }
    }
    return attacked;
}
void Knight::updateValidMovesWhilePinned() {
    // if knight is pinned, it can never move
    this->validMoves->clear();
}

void Bishop::updateMoves() {
    for (auto move : bishopMoves) {
        int dx = this->position.x;
        int dy = this->position.y;
        bool breakInner = false;
        dx += move[0];
        dy += move[1];
        //for cycle runs until coordinates are out of bounds
        for (; !this->board->coordOutOfBounds(Square(dx, dy)); dx += move[0], dy += move[1]) {
            if (breakInner) continue;
            Square newPos(dx, dy);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if (p != nullptr && p->getColor() == this->getColor()) {
                breakInner = true;
                continue;
            }
            this->moves->push_back(newPos);
        }
    }
}

void Bishop::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
    for (auto move : bishopMoves) {
        int dx = this->position.x;
        int dy = this->position.y;
        bool breakInner = false;
        dx += move[0];
        dy += move[1];
        //for cycle runs until coordinates are out of bounds
        for (; !this->board->coordOutOfBounds(Square(dx, dy)); dx += move[0], dy += move[1]) {
            if (breakInner) break;
            Square newPos(dx, dy);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if (p != nullptr) {
                breakInner = true;
                if (p->getColor() == this->getColor()) {
                    p->setProtectedBy(this);
                    continue;
                } else {
                    if (this->board->checkPinnedPiece(this, p.get())) {
                        p->setPinnedBy(this);
                        p->updateValidMovesWhilePinned();
                    }
                }
            }
            if (!onlyPinsAndProtects) this->validMoves->push_back(newPos);
        }
    }
}

vector<Square> Bishop::getSquaresAttacked() const {
    vector<Square> attacked;
    for (auto move: bishopMoves) {
        int dx = this->position.x;
        int dy = this->position.y;
        dx += move[0];
        dy += move[1];
        //for cycle runs until coordinates are out of bounds
        for (; !this->board->coordOutOfBounds(Square(dx, dy)); dx += move[0], dy += move[1]) {
            Square newPos(dx, dy);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            attacked.push_back(newPos);
            // it is necessary to add squares behind enemy king so it knows it is still in check
            // if it moves one square away from the checker
            if ((p != nullptr && p->getKind() != KING)
                || (p != nullptr && p->getKind() == KING && p->getColor() == this->getColor())) {
                break;
            }
        }
    }
    return attacked;
}

void Bishop::updateValidMovesWhilePinned() {
    this->validMoves->clear();
    int dx = this->position.x;
    int dy = this->position.y;

    Square pinnerPosition = this->getPinnedBy()->getPosition();
    // if pinned vertically or horizontally, no possible moves for bishop
    if (dx == pinnerPosition.x || dy == pinnerPosition.y) return;

    int incrementX = dx < pinnerPosition.x ? 1 : -1;
    int incrementY = dy < pinnerPosition.y ? 1 : -1;

    dx += incrementX;
    dy += incrementY;

    for (; !(dx == pinnerPosition.x && dy == pinnerPosition.y);
           dx += incrementX, dy += incrementY) {
        Square newPos(dx, dy);
        this->validMoves->push_back(newPos);
    }

    this->validMoves->push_back(pinnerPosition);
}

void Queen::updateMoves() {
    for (auto move: queenKingMoves) {
        int dx = this->position.x;
        int dy = this->position.y;
        bool breakInner = false;
        dx += move[0];
        dy += move[1];
        //for cycle runs until coordinates are out of bounds
        for (; !this->board->coordOutOfBounds(Square(dx, dy)); dx += move[0], dy += move[1]) {
            if (breakInner) continue;
            Square newPos(dx, dy);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if (p != nullptr && p->getColor() == this->getColor()) {
                breakInner = true;
                continue;
            }
            this->moves->push_back(newPos);
        }
    }
}

void Queen::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
    for (auto move: queenKingMoves) {
        int dx = this->position.x;
        int dy = this->position.y;
        bool breakInner = false;
        dx += move[0];
        dy += move[1];
        //for cycle runs until coordinates are out of bounds
        for (; !this->board->coordOutOfBounds(Square(dx, dy)); dx += move[0], dy += move[1]) {
            if (breakInner) break;
            Square newPos(dx, dy);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if (p != nullptr) {
                breakInner = true;
                if (p->getColor() == this->getColor()) {
                    p->setProtectedBy(this);
                    continue;
                } else {
                    if (this->board->checkPinnedPiece(this, p.get())) {
                        p->setPinnedBy(this);
                        p->updateValidMovesWhilePinned();
                    }
                }
            }
            if (!onlyPinsAndProtects) this->validMoves->push_back(newPos);
        }
    }
}

vector<Square> Queen::getSquaresAttacked() const {
    vector<Square> v;
    for (auto move: queenKingMoves) {
        int dx = this->position.x;
        int dy = this->position.y;
        dx += move[0];
        dy += move[1];
        //for cycle runs until coordinates are out of bounds
        for (; !this->board->coordOutOfBounds(Square(dx, dy)); dx += move[0], dy += move[1]) {
            Square newPos(dx, dy);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            v.push_back(newPos);
            // it is necessary to add squares behind enemy king so it knows it is still in check
            // if it moves one square away from the checker
            if ((p != nullptr && p->getKind() != KING)
                || (p != nullptr && p->getKind() == KING && p->getColor() == this->getColor())) {
                break;
            }
        }
    }
    return v;
}

void Queen::updateValidMovesWhilePinned() {
    this->validMoves->clear();
    int dx = this->position.x;
    int dy = this->position.y;

    Square pinnerPosition = this->getPinnedBy()->getPosition();

    int incrementX = dx == pinnerPosition.x ? 0 : dx < pinnerPosition.x ? 1 : -1;
    int incrementY = dy == pinnerPosition.y ? 0 : dy < pinnerPosition.y ? 1 : -1;

    dx += incrementX;
    dy += incrementY;

    for (; !(dx == pinnerPosition.x && dy == pinnerPosition.y);
           dx += incrementX, dy += incrementY) {
        Square newPos(dx, dy);
        this->validMoves->push_back(newPos);
    }
    this->validMoves->push_back(pinnerPosition);
}

void King::updateMoves() {
    for (auto move: queenKingMoves) {
        Square newPos(this->position.x + move[0], this->position.y + move[1]);
        if (!this->board->coordOutOfBounds(newPos)) {
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if (p != nullptr && (p->getColor() == this->getColor() || p->getKind() == 'k'))
                continue;
            this->moves->push_back(newPos);
        }
    }
    // castle
    if (!this->hasMoved() && this->board->getPlayerInCheck() != this->getColor()
        && this->board->canCastle()) {
        auto rooks = this->board->getPiecesOfColorAndKind(this->color, ROOK);
        for (const auto &rook: rooks) {
            if (!rook->hasMoved()) {
                vector<Square> squaresBetween = this->board->getSquaresBetween(rook.get(), this);
                if (squaresBetween.size() == 2) {
                    //short castle
                    bool pieceBetween = false;
                    //check if there are pieces in the way
                    for (Square square: squaresBetween) {
                        if (this->board->getPieceOnCoords(square) != nullptr) {
                            pieceBetween = true;
                            break;
                        }
                    }
                    if (pieceBetween) continue;

                    int dx = rook->getPosition().x > this->position.x ? rook->getPosition().x - 1 :
                             rook->getPosition().x + 1;
                    Square newPos(dx, this->position.y);
                    this->moves->push_back(newPos);
                } else {
                    //long castle
                    bool pieceBetween = false;
                    //check if there are pieces in the way
                    for (Square square: squaresBetween) {
                        if (this->board->getPieceOnCoords(square) != nullptr) {
                            pieceBetween = true;
                            break;
                        }
                    }
                    if (pieceBetween) continue;

                    int dx = rook->getPosition().x > this->position.x ? rook->getPosition().x - 2 :
                             rook->getPosition().x + 2;
                    Square newPos(dx, this->position.y);
                    this->moves->push_back(newPos);
                }
            }
        }
    }
}

void King::updateValidMovesPinsProtects(bool) {
    for (auto move: queenKingMoves) {
        Square newPos(this->position.x + move[0], this->position.y + move[1]);
        if (this->board->coordOutOfBounds(newPos)
            || this->board->isSquareUnderAttack(invertColor(this->getColor()), newPos)) {
            continue;
        }
        shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
        if (p != nullptr && p->getColor() == this->getColor()) {
            p->setProtectedBy(this);
            continue;
        }
        if (p != nullptr && p->getProtectedBy() != nullptr) continue;
        this->validMoves->push_back(newPos);
    }
    // castle
    if (!this->hasMoved() && this->board->getPlayerInCheck() != this->getColor()
        && this->board->canCastle()) {
        auto rooks = this->board->getPiecesOfColorAndKind(this->color, ROOK);
        for (const auto &rook: rooks) {
            if (!rook->hasMoved()) {
                vector<Square> squaresBetween = this->board->getSquaresBetween(rook.get(), this);
                bool squareAttacked = false;
                for (Square &square: squaresBetween) {
                    if (this->board->isSquareUnderAttack(invertColor(this->getColor()), square)) {
                        squareAttacked = true;
                        break;
                    }
                }
                if (squareAttacked) continue;
                if (squaresBetween.size() == 2) {
                    //short castle
                    bool pieceBetween = false;
                    //check if there are pieces in the way
                    for (Square square: squaresBetween) {
                        if (this->board->getPieceOnCoords(square) != nullptr) {
                            pieceBetween = true;
                            break;
                        }
                    }
                    if (pieceBetween) continue;

                    int dx = rook->getPosition().x > this->position.x ? rook->getPosition().x - 1 :
                             rook->getPosition().x + 1;
                    Square newPos(dx, this->position.y);
                    this->validMoves->push_back(newPos);
                } else {
                    //long castle

                    bool pieceBetween = false;
                    //check if there are pieces in the way
                    for (Square square: squaresBetween) {
                        if (this->board->getPieceOnCoords(square) != nullptr) {
                            pieceBetween = true;
                            break;
                        }
                    }
                    if (pieceBetween) continue;

                    int dx = rook->getPosition().x > this->position.x ? rook->getPosition().x - 2 :
                             rook->getPosition().x + 2;
                    Square newPos(dx, this->position.y);
                    this->validMoves->push_back(newPos);
                }
            }
        }
    }
}

vector<Square> King::getSquaresAttacked() const {
    vector<Square> attacked;
    for (auto move: queenKingMoves) {
        Square newPos(this->position.x + move[0], this->position.y + move[1]);
        if (this->board->coordOutOfBounds(newPos)) {
            continue;
        }
        attacked.push_back(newPos);
    }
    return attacked;
}

void King::updateValidMovesWhilePinned() {
    unreachable("King can never be pinned -> check");
}
} // namespace GTLib2::domain::chess

namespace GTLib2::domains {
using chess::Square;

KriegspielAction::KriegspielAction
    (ActionId id, pair<shared_ptr<AbstractPiece>, Square> move, Square originalPos) :
    Action(id), moveFrom(originalPos), move_(std::move(move)) {}

KriegspielAction::KriegspielAction(ActionId id) :
    Action(id), moveFrom(-1, -1),
    move_(pair<shared_ptr<AbstractPiece>, Square>(nullptr, Square(-1, -1))) {}

bool KriegspielAction::operator==(const GTLib2::Action &that) const {
    const auto rhsAction = dynamic_cast<const KriegspielAction *>(&that);
    return this->move_.first->equal_to(*rhsAction->move_.first)
        && this->move_.second == rhsAction->move_.second
        && this->movingFrom() == rhsAction->movingFrom();
}

HashType KriegspielAction::getHash() const {
    string s = this->move_.first->toString() + chess::coordToString(this->move_.second)
        + chess::coordToString(this->movingFrom());
    return hashWithSeed(s.data(), s.size() * sizeof(uint32_t), 16484135155);
}

shared_ptr<KriegspielAction> KriegspielAction::clone() const {
    Square s = Square(this->getMove().second.x, this->getMove().second.y);
    shared_ptr<AbstractPiece> p = this->getMove().first->clone();
    Square f = Square(this->movingFrom().x, this->movingFrom().y);
    auto a =
        make_shared<KriegspielAction>(id_, pair<shared_ptr<AbstractPiece>, chess::Square>(p, s), f);
    return a;
}

pair<shared_ptr<AbstractPiece>, Square> KriegspielAction::getMove() const {
    return this->move_;
}

Square KriegspielAction::movingFrom() const {
    return this->moveFrom;
}

KriegspielDomain::KriegspielDomain(unsigned int maxDepth,
                                   unsigned int legalMaxDepth,
                                   chess::BOARD board)
    : Domain(maxDepth, 2, true,
             make_shared<KriegspielAction>(),
             make_shared<KriegspielObservation>()) {
    vector<double> rewards(2, 0.0);
    Outcome rootState(make_shared<KriegspielState>(this, legalMaxDepth, board),
                      {make_shared<Observation>(NO_OBSERVATION),
                       make_shared<Observation>(NO_OBSERVATION)},
                      make_shared<Observation>(NO_OBSERVATION),
                      rewards);
    rootStatesDistribution_.push_back(OutcomeEntry(rootState));
    maxUtility_ = 1;
}

KriegspielDomain::KriegspielDomain(unsigned int maxDepth, unsigned int legalMaxDepth, string s)
    : Domain(maxDepth,
             2,
             true,
             make_shared<KriegspielAction>(),
             make_shared<KriegspielObservation>()) {
    vector<double> rewards(2);
    vector<shared_ptr<Observation>>
        Obs{make_shared<Observation>(NO_OBSERVATION), make_shared<Observation>(-1)};
    Outcome o(make_shared<KriegspielState>(this, legalMaxDepth, s),
              Obs,
              shared_ptr<Observation>(),
              rewards);
    rootStatesDistribution_.push_back(OutcomeEntry(o));
}

KriegspielState::KriegspielState(const Domain *domain, unsigned int legalMaxDepth, chess::BOARD b) :
    State(domain, hashCombine(56456654521424531, legalMaxDepth, int(b))),
    moveHistory_(make_shared<vector<shared_ptr<KriegspielAction>>>()),
    attemptedMoveHistory_(make_shared<vector<shared_ptr<KriegspielAction>>>()),
    playerOnTheMove_(chess::WHITE),
    enPassantSquare_(-1, -1),
    legalMaxDepth_(legalMaxDepth) {
    this->initBoard(b);
    this->updateState(chess::WHITE);
}

KriegspielState::KriegspielState(const Domain *domain, unsigned int legalMaxDepth, string s)
    : State(domain, hashCombine(6545315341531531, legalMaxDepth, s)),
      moveHistory_(make_shared<vector<shared_ptr<KriegspielAction>>>()),
      attemptedMoveHistory_(make_shared<vector<shared_ptr<KriegspielAction>>>()),
      playerOnTheMove_(chess::WHITE),
      enPassantSquare_(-1, -1),
      legalMaxDepth_(legalMaxDepth) {
    this->initBoard(std::move(s));
    this->updateAllPieces();
}

KriegspielState::KriegspielState(const Domain *domain, unsigned int legalMaxDepth, int x, int y,
                                 shared_ptr<vector<shared_ptr<chess::AbstractPiece>>> pieces,
                                 Square enPassantSquare,
                                 const shared_ptr<vector<shared_ptr<KriegspielAction>>> &moves,
                                 int p, bool castle,
                                 shared_ptr<vector<shared_ptr<KriegspielAction>>> attemptedMoves)
// todo: hash is not computed from shared_ptr<vec<shared_ptr>> as there is not support for that atm
    : State(domain, hashCombine(3156841351231, enPassantSquare, x, y, legalMaxDepth, p, castle)),
      pieces_(std::move(pieces)), moveHistory_(move(moves)),
      attemptedMoveHistory_(std::move(attemptedMoves)), playerOnTheMove_(p),
      enPassantSquare_(enPassantSquare), legalMaxDepth_(legalMaxDepth), xSize_(x), ySize_(y),
      canPlayerCastle_(castle) {

    //rebind to new board state
    for (shared_ptr<AbstractPiece> &piece: *this->pieces_) {
        piece->setBoard(this);
    }
}


// todo: this is only hotfix
unsigned long KriegspielState::countAvailableActionsFor(Player player) const {
    return getAvailableActionsFor(player).size();
}

vector<shared_ptr<Action>> KriegspielState::getAvailableActionsFor(Player player) const {
    vector<shared_ptr<Action>> actions;
    if (player != this->playerOnTheMove_
        || this->gameHasEnded_
        || this->moveHistory_->size() == this->legalMaxDepth_) {
        return actions;
    }
    int count = 0;
    for (const auto &piece: this->getPiecesOfColor(player)) {
        for (Square move: *piece->getAllMoves()) {
            auto newAction = make_shared<KriegspielAction>(
                count, pair<shared_ptr<AbstractPiece>, Square>(piece, move),
                piece->getPosition());
            if (std::find_if(attemptedMoveHistory_->begin(), attemptedMoveHistory_->end(),
                             [&newAction](const shared_ptr<KriegspielAction> &action) {
                               return *action == *newAction;
                             }) == attemptedMoveHistory_->end()) {
                actions.push_back(newAction);
                ++count;
            }
        }
    }
    return actions;
}
// todo: do i need to iterate over pointers?
void KriegspielState::promote(shared_ptr<AbstractPiece> p, Square pos) const {
    auto queen = make_shared<chess::Queen>(chess::QUEEN, p->getColor(), pos, this);
    for (auto It = this->pieces_->begin(); It != this->pieces_->end();) {
        if (*(*It) == *p) {
            this->pieces_->erase(It);
            this->pieces_->push_back(queen);
            break;
        } else {
            ++It;
        }
    }
}

vector<shared_ptr<AbstractPiece>> KriegspielState::getPiecesOfColor(int color) const {
    vector<shared_ptr<AbstractPiece>> pieces;
    for (auto &p: *this->pieces_) {
        if (p->getColor() == color) {
            pieces.push_back(p);
        }
    }
    return pieces;
}

int KriegspielState::getPlayerInCheck() const {
    return this->playerInCheck_;
}

vector<shared_ptr<AbstractPiece>> KriegspielState::getCheckingFigures() const {
    return this->checkingFigures_;
}

void KriegspielState::checkPlayerInCheck() {
    int kingColor = this->playerOnTheMove_;
    this->playerInCheck_= NO_PLAYER;
    this->checkingFigures_.clear();
    auto king = this->getPiecesOfColorAndKind(kingColor, chess::KING)[0];
    Square kingPos = king->getPosition();
    for (const auto &piece: this->getPiecesOfColor(chess::invertColor(kingColor))) {
        vector<Square> attackSquares = piece->getSquaresAttacked();
        if (std::find(attackSquares.begin(), attackSquares.end(), kingPos) != attackSquares.end()) {
            this->playerInCheck_= kingColor;
            this->checkingFigures_.push_back(piece);
        }
    }
}

int KriegspielState::checkGameOverCheck(int playerColor) const {
    int inCheck = NO_PLAYER;
    auto king = this->getPiecesOfColorAndKind(playerColor, chess::KING)[0];
    Square kingPos = king->getPosition();
    for (const auto &piece: this->getPiecesOfColor(chess::invertColor(playerColor))) {
        vector<Square> attackSquares = piece->getSquaresAttacked();
        if (std::find(attackSquares.begin(), attackSquares.end(), kingPos) != attackSquares.end()) {
            inCheck = king->getColor();
        }
    }
    return inCheck;
}

bool KriegspielState::canCastle() const {
    return this->canPlayerCastle_;
}
// TODO finished here
void KriegspielState::castle(KriegspielAction *a) const {
    pair<shared_ptr<AbstractPiece>, Square> move = a->getMove();
    shared_ptr<AbstractPiece> king = move.first;
    Square pos = move.second;
    int dx = king->getPosition().x;
    int dy = king->getPosition().y;
    //get rook in the direction of the move so we don't have to make special cases for other boards and black/white board asymmetry
    int incrementX = pos.x > dx ? 1 : -1;
    shared_ptr<AbstractPiece> rook;
    dx += incrementX;
    for (; ((rook = this->getPieceOnCoords(Square(dx, dy))) == nullptr); dx += incrementX);
    int newRookX = pos.x > rook->getPosition().x ? pos.x + 1 : pos.x - 1;
    Square newRookPos(newRookX, rook->getPosition().y);
    rook->move(newRookPos);
}

bool KriegspielState::makeMove(KriegspielAction *a) {
    pair<shared_ptr<AbstractPiece>, Square> move = a->getMove();
    shared_ptr<AbstractPiece> p = this->getPieceOnCoords(move.first->getPosition());
    Square pos = move.second;

    if (std::find(p->getAllValidMoves()->begin(), p->getAllValidMoves()->end(), pos)
        != p->getAllValidMoves()->end()) {

        if (p->getKind() == 'k') {
            auto k2 =
                this->getPiecesOfColorAndKind(chess::invertColor(p->getColor()), chess::KING)[0];
            auto attackSquares = k2->getAllMoves();
            if (std::find(attackSquares->begin(), attackSquares->end(), a->getMove().second)
                != attackSquares->end())
                return false;
        }
        //move valid
        shared_ptr<AbstractPiece> checkCapture = this->getPieceOnCoords(pos);
        if (checkCapture != nullptr) {
            //capturedPawn
            this->capturedPiece = checkCapture;
            for (auto It = this->pieces_->begin(); It != this->pieces_->end();) {
                if (*(*It) == *checkCapture) {
                    this->pieces_->erase(It);
                    break;
                } else {
                    ++It;
                }
            }
        }

        if (pos.x == this->enPassantSquare_.x && pos.y == this->enPassantSquare_.y) {
            int pawnColor = chess::invertColor(p->getColor());
            int y = pawnColor == chess::WHITE ? pos.y + 1 : pos.y - 1;
            Square enPassPos(pos.x, y);
            shared_ptr<AbstractPiece> enPassPawn = this->getPieceOnCoords(enPassPos);
            if (enPassPawn != nullptr) {
                for (auto It = this->pieces_->begin(); It != this->pieces_->end();) {
                    if (*(*It) == *enPassPawn) {
                        this->pieces_->erase(It);
                        break;
                    } else {
                        ++It;
                    }
                }
            }
        }

        if (p->getKind() == chess::KING && abs(a->movingFrom().x - pos.x) > 1) {
            this->castle(a);
        }
        p->move(pos);
        this->checkPlayerInCheck();
        if (p->getKind() == chess::PAWN
            && ((pos.y == this->getYSize() && p->getColor() == chess::WHITE)
                || (pos.y == 1 && p->getColor() == chess::BLACK))) {
            this->promote(p, pos);
        }
        return true;
    } else {
        return false;
    }
}

int KriegspielState::checkGameOver() const {
    int result = 0; // 0 = play continue, 1 = player on the move lost, 2 = draw
    vector<shared_ptr<AbstractPiece>> whitePieces = this->getPiecesOfColor(0);
    vector<shared_ptr<AbstractPiece>> blackPieces = this->getPiecesOfColor(1);
    if (whitePieces.size() <= 2 && blackPieces.size() <= 2) {
        bool isDraw = true;
        whitePieces.insert(whitePieces.end(), blackPieces.begin(), blackPieces.end());
        for (const shared_ptr<AbstractPiece> &p: whitePieces) {
            if (p->getKind() == chess::KING) continue;
            else if (p->getKind() == chess::PAWN || p->getKind() == chess::ROOK
                || p->getKind() == chess::QUEEN)
                isDraw = false;
        }
        if (isDraw) {
            result = 2;
            return result;
        }
    }

    bool hasMoves[] = {false, false};
    int isCheck = this->checkGameOverCheck(this->playerOnTheMove_);

    for (const shared_ptr<AbstractPiece> &p: *this->pieces_) {
        hasMoves[p->getColor()] = hasMoves[p->getColor()] ? true : !p->getAllValidMoves()->empty();
    }

    if (isCheck == this->playerOnTheMove_) {
        if (!hasMoves[this->playerOnTheMove_]) {
            result = 1;
        } else if (this->moveHistory_->size() == this->legalMaxDepth_) {
            result = 2;
        } else if (this->moveHistory_->size() + this->attemptedMoveHistory_->size()
            == domain_->getMaxStateDepth()) {
            result = 2;
        }
    } else if (!hasMoves[this->playerOnTheMove_]) { //stale-mate
        result = 2;
    } else if (this->moveHistory_->size() == this->legalMaxDepth_) {
        result = 2;
    }

    return result;
}

Square KriegspielState::getEnPassantSquare() const {
    return this->enPassantSquare_;
}

OutcomeDistribution KriegspielState::performActions(
    const vector<shared_ptr<Action>> &actions) const {
    auto a1 = dynamic_cast<KriegspielAction *>(actions[0].get());
    auto a2 = dynamic_cast<KriegspielAction *>(actions[1].get());
    vector<shared_ptr<Observation>> observations(2);
    shared_ptr<Observation> publicObservation;
    vector<double> rewards(2);
    shared_ptr<KriegspielState> s;
    int playerOnMove = this->playerOnTheMove_;
    Square enPassSquare(-1, -1);
    KriegspielAction *a = a1->getMove().first != nullptr ? a1 : a2;
    shared_ptr<vector<shared_ptr<AbstractPiece>>> pieces = this->copyPieces();
    shared_ptr<vector<shared_ptr<KriegspielAction>>> history = this->copyMoveHistory();
    shared_ptr<vector<shared_ptr<KriegspielAction>>> attemptedmoves = this->copyAttemptedMoves();
    s = make_shared<KriegspielState>(domain_,
                                     this->legalMaxDepth_,
                                     this->xSize_,
                                     this->ySize_,
                                     pieces,
                                     this->enPassantSquare_,
                                     history,
                                     playerOnMove,
                                     this->canPlayerCastle_,
                                     attemptedmoves);
    s->updateState(playerOnMove);
    shared_ptr<KriegspielAction> ac = a->clone();
    if (s->makeMove(a)) {
        s->addToHistory(ac);
        enPassSquare = this->checkEnPassant(a);
        playerOnMove = chess::invertColor(playerOnMove);
        s->setPlayerOnMove(playerOnMove);
        s->setEnPassant(enPassSquare);
        s->updateState(playerOnMove);
        publicObservation = s->calculatePublicObservation();
        observations[this->playerOnTheMove_] = publicObservation;
        observations[chess::invertColor(this->playerOnTheMove_)] = publicObservation;

    } else {
        s->addToAttemptedMoves(ac);
        s->updateState(playerOnMove); //stalemate check
        observations[this->playerOnTheMove_] = make_shared<KriegspielObservation>(a->getId());
        observations[chess::invertColor(this->playerOnTheMove_)] =
            make_shared<KriegspielObservation>(NO_OBSERVATION);
        publicObservation = make_shared<KriegspielObservation>(NO_OBSERVATION);

    }
    switch (s->checkGameOver()) {
        case 0:
            rewards[0] = 0.0;
            rewards[1] = 0.0;
            break;
        case 1:
            rewards[s->playerOnTheMove_] = -1.0;
            rewards[chess::invertColor(s->playerOnTheMove_)] = 1.0;
            s->setGameHasEnded(true);
            break;
        case 2:
            rewards[0] = 0.0;
            rewards[1] = 0.0;
            s->setGameHasEnded(true);
            break;
        default:unreachable("wrong return value of checkGameOver()!");
    }

    Outcome o(s, observations, publicObservation, rewards);
    OutcomeDistribution prob;
    prob.push_back(OutcomeEntry(o));
    return prob;
}

Square KriegspielState::checkEnPassant(KriegspielAction *a) const {
    shared_ptr<AbstractPiece> abstractPiece = a->getMove().first;
    Square pos(-1, -1);
    if (abstractPiece->getKind() != chess::PAWN) {
        return pos;
    } else {
        if ((a->getMove().second.y - a->movingFrom().y) == 2) {
            pos.x = a->movingFrom().x;
            pos.y = a->movingFrom().y + 1;
            return pos;
        } else if ((a->getMove().second.y - a->movingFrom().y) == -2) {
            pos.x = a->movingFrom().x;
            pos.y = a->movingFrom().y - 1;
            return pos;
        } else {
            return pos;
        }
    }
}

bool KriegspielState::operator==(const GTLib2::State &rhs) const {
    const auto rhsState = dynamic_cast<const KriegspielState *>(&rhs);
    if (this->xSize_ != rhsState->xSize_ || this->ySize_ != rhsState->ySize_) {
        return false;
    }
    for (int i = 1; i <= this->xSize_; i++) {
        for (int j = 1; j <= this->ySize_; j++) {
            shared_ptr<AbstractPiece> p1 = this->getPieceOnCoords(Square(i, j));
            shared_ptr<AbstractPiece> p2 = this->getPieceOnCoords(Square(i, j));
            if (p1 == nullptr && p2 == nullptr) continue;
            if ((p1 != nullptr && p2 == nullptr) || (p1 == nullptr && p2 != nullptr)) return false;
            if (!(*p1 == *p2)) return false;
        }
    }
    return true;
}

string KriegspielState::toString() const {
    //DEPRECATED
    vector<vector<string>> board;
    for (int j = 0; j < this->ySize_; j++) {
        vector<string> row;
        for (int i = 0; i < this->xSize_; i++) {
            row.emplace_back("_");
        }
        board.push_back(row);
    }

    for (shared_ptr<AbstractPiece> p: *this->pieces_) {
        Square pos = p->getPosition();
        pos.y -= 1;
        pos.x -= 1;
        board.at(pos.y).at(pos.x) = p->toString();
    }

    string s;
    for (int i = this->ySize_ - 1; i >= 0; i--) {
        s += to_string(i + 1) + " ";
        vector<string> row = board.at(i);
        for (const string &str: row) {
            s += str;
        }
        s += "\n";
    }

    s += "  ";
    for (int i = 0; i < this->xSize_; i++) s += 'a' + i;
    s += "\n";
    if (!this->attemptedMoveHistory_->empty()) {
        s += "\nPreviously attempted illegal moves: [";
        for (auto action : *this->attemptedMoveHistory_) {
            s += action->toString();
            s += ", ";
        }
        s += "]\n";
    }
    return s;

    /*chess::FenBoardFactory f;
    return f.createFEN(this);*/
}

shared_ptr<AbstractPiece> KriegspielState::getPieceOnCoords(Square pos) const {
    for (const shared_ptr<AbstractPiece> &p: *this->pieces_) {
        if (p->getPosition() == pos) {
            return p;
        }
    }
    return nullptr;
}

bool KriegspielState::coordOutOfBounds(Square pos) const {
    return pos.x > this->xSize_ || pos.y > this->ySize_ || pos.x < 1 || pos.y < 1;
}

void KriegspielState::updateAllPieces() const {
    for (shared_ptr<AbstractPiece> &p: *this->pieces_) {
        p->update();
    }
}

void KriegspielState::resetAllPieces() {
    for (shared_ptr<AbstractPiece> &p: *this->pieces_) {
        p->reset();
    }
}

void KriegspielState::updatePiecesOfColor(int color) const {
    for (shared_ptr<AbstractPiece> &piece: this->getPiecesOfColor(color)) {
        piece->update();
    }
}

vector<shared_ptr<AbstractPiece>>
KriegspielState::getPiecesOfColorAndKind(int c, chess::pieceName n) const {
    vector<shared_ptr<AbstractPiece>> v;
    for (const shared_ptr<AbstractPiece> &p: *this->pieces_) {
        if (p->getColor() == c && p->getKind() == n) {
            v.push_back(p);
        }
    }
    return v;
}

bool KriegspielState::isSquareUnderAttack(int color, Square square) const {
    vector<shared_ptr<AbstractPiece>> pieces = this->getPiecesOfColor(color);
    for (const shared_ptr<AbstractPiece> &p: pieces) {
        vector<Square> attackedSquares = p->getSquaresAttacked();
        if (std::find(attackedSquares.begin(), attackedSquares.end(), square)
            != attackedSquares.end()) {
            return true;
        }
    }
    return false;
}

void KriegspielState::clearBoard() {
    this->pieces_->clear();
}

void KriegspielState::insertPiece(shared_ptr<AbstractPiece> p) {
    this->pieces_->push_back(p);
}

vector<Square>
KriegspielState::getSquaresPierced(AbstractPiece *pierced, AbstractPiece *piercer) const {
    Square posPiercer = piercer->getPosition();
    Square posPierced = pierced->getPosition();
    vector<Square> squares;

    int dx = posPierced.x;
    int dy = posPierced.y;

    int incrementX = dx == posPiercer.x ? 0 : dx < posPiercer.x ? -1 : 1;
    int incrementY = dy == posPiercer.y ? 0 : dy < posPiercer.y ? -1 : 1;

    dx += incrementX;
    dy += incrementY;

    for (; !this->coordOutOfBounds(Square(dx, dy)); dx += incrementX, dy += incrementY) {
        Square newPos(dx, dy);
        squares.push_back(newPos);
    }
    return squares;
}

vector<Square> KriegspielState::getSquaresBetween(AbstractPiece *p1, AbstractPiece *p2) const {

    Square pos1 = p1->getPosition();
    Square pos2 = p2->getPosition();
    vector<Square> squares;

    int dx = pos1.x;
    int dy = pos1.y;

    int incrementX = dx == pos2.x ? 0 : dx < pos2.x ? 1 : -1;
    int incrementY = dy == pos2.y ? 0 : dy < pos2.y ? 1 : -1;

    dx += incrementX;
    dy += incrementY;

    for (; !(dx == pos2.x && dy == pos2.y); dx += incrementX, dy += incrementY) {
        Square newPos(dx, dy);
        if (this->coordOutOfBounds(newPos)) {
            string s1 = "Position 1: " + to_string(p1->getPosition().x) + ", "
                + to_string(p1->getPosition().y) + "\r\n"
                                                   "Figure 1: " + p1->toString() + "\r\n";
            string s2 = "Position 2: " + to_string(p2->getPosition().x) + ", "
                + to_string(p2->getPosition().y) + "\r\n"
                                                   "Figure 2: " + p2->toString() + "\r\n";
            cout << s1;
            cout << s2;
            throw std::invalid_argument("wrong figures");
        }
        squares.push_back(newPos);
    }

    return squares;
}

bool KriegspielState::checkPinnedPiece(AbstractPiece *pinner, AbstractPiece *pinned) const {
    int kingColor = pinned->getColor();
    shared_ptr<AbstractPiece>
        king = this->getPiecesOfColorAndKind(kingColor, chess::pieceName::KING)[0];
    vector<Square> squaresToCheck = this->getSquaresPierced(pinned, pinner);
    for (Square i: squaresToCheck) {
        shared_ptr<AbstractPiece> p = this->getPieceOnCoords(i);
        if (p != nullptr) {
            return p == king;
        }
    }
    return false;
}

void KriegspielState::initBoard(chess::BOARD b) {
    chess::BoardFactory bf;
    chess::boardInfo binfo = bf.create(b, this);
    this->pieces_ = binfo.pieces;
    this->xSize_= binfo.x;
    this->ySize_ = binfo.y;
    this->canPlayerCastle_ = binfo.castle;
}

int KriegspielState::getXSize() const {
    return this->xSize_;
}

int KriegspielState::getYSize() const {
    return this->ySize_;
}

void KriegspielState::setPlayerOnMove(int p) {
    this->playerOnTheMove_ = p;
}

void KriegspielState::setGameHasEnded(bool gameHasEnded) {
    this->gameHasEnded_ = gameHasEnded;
}

shared_ptr<vector<shared_ptr<AbstractPiece>>> KriegspielState::copyPieces() const {
    shared_ptr<vector<shared_ptr<AbstractPiece>>>
        v = make_shared<vector<shared_ptr<AbstractPiece>>>();
    for (const shared_ptr<AbstractPiece> &p: *this->pieces_) {
        auto asdf = p->clone();
        v->push_back(asdf);
    }
    return v;
}

shared_ptr<vector<shared_ptr<KriegspielAction>>> KriegspielState::copyMoveHistory() const {
    shared_ptr<vector<shared_ptr<KriegspielAction>>>
        v = make_shared<vector<shared_ptr<KriegspielAction>>>();
    for (const shared_ptr<KriegspielAction> &a: *this->moveHistory_) {
        auto asdf = a->clone();
        v->push_back(asdf);
    }
    return v;
}

void KriegspielState::addToHistory(shared_ptr<KriegspielAction> a) {
    this->moveHistory_->push_back(a);
    this->attemptedMoveHistory_->clear();
}

void KriegspielState::setEnPassant(chess::Square s) {
    this->enPassantSquare_ = s;
    //refresh moves on pawns after setting enpassant square
    vector<shared_ptr<AbstractPiece>>
        pawns = this->getPiecesOfColorAndKind(this->playerOnTheMove_, chess::PAWN);
    for (shared_ptr<AbstractPiece> &p: pawns) {
        p->update();
    }
}

void KriegspielState::updateState(int p) {
    this->playerOnTheMove_ = p;
    this->resetAllPieces();
    this->updatePiecesOfColor(chess::invertColor(this->playerOnTheMove_));
    this->checkPlayerInCheck();
    this->updatePiecesOfColor(this->playerOnTheMove_);

    //update kings again to remove moves that would cut protected pieces
    shared_ptr<AbstractPiece>
        whiteKing = this->getPiecesOfColorAndKind(chess::WHITE, chess::KING)[0];
    shared_ptr<AbstractPiece>
        blackKing = this->getPiecesOfColorAndKind(chess::BLACK, chess::KING)[0];
    whiteKing->reset();
    blackKing->reset();
    whiteKing->update();
    blackKing->update();
}

void KriegspielState::initBoard(string s) {
    chess::FenBoardFactory bf;
    chess::boardInfo binfo = bf.create(std::move(s), this);
    this->pieces_ = binfo.pieces;
    this->xSize_= binfo.x;
    this->ySize_ = binfo.y;
    this->canPlayerCastle_ = binfo.castle;
    if (binfo.xEnpass) {
        Square sq(binfo.xEnpass, binfo.yEnpass);
        this->setEnPassant(sq);
    }
}

shared_ptr<vector<shared_ptr<KriegspielAction>>> KriegspielState::copyAttemptedMoves() const {
    shared_ptr<vector<shared_ptr<KriegspielAction>>>
        v = make_shared<vector<shared_ptr<KriegspielAction>>>();
    for (const shared_ptr<KriegspielAction> &a: *this->attemptedMoveHistory_) {
        auto asdf = a->clone();
        v->push_back(asdf);
    }
    return v;
}

void KriegspielState::addToAttemptedMoves(shared_ptr<KriegspielAction> a) {
    this->attemptedMoveHistory_->push_back(a);
}

shared_ptr<KriegspielObservation> KriegspielState::calculatePublicObservation() const {
    ObservationId observation = 0; // if any pieces captured, otherwise 0
    string capturedP;
    if (this->capturedPiece != nullopt) {
        AbstractPiece *p = this->capturedPiece->get();
        if (p->getKind() == 'p') {
            observation = p->getPosition().x + (p->getPosition().y - 1) * this->ySize_;
            capturedP = "pawn at " + coordToString(p->getPosition());
        } else { // captured other piece
            observation = 64 + p->getPosition().x + (p->getPosition().y - 1) * this->ySize_;
            capturedP = "piece at " + coordToString(p->getPosition());
        }
    }
    vector<chess::checkType> checks;
    if (this->getPlayerInCheck() == this->playerOnTheMove_) {
        Square kingPosition =
            this->getPiecesOfColorAndKind(this->playerOnTheMove_, chess::KING)[0]->getPosition();
        int rowMid = this->xSize_/ 2;
        int colMid = this->ySize_ / 2;
        for (shared_ptr<AbstractPiece> piece : this->getCheckingFigures()) {
            Square piecePos = piece->getPosition();
            if (piece->getKind() == 'n') {
                // it is knight
                observation |= 1UL << 27;
                checks.emplace_back(chess::KNIGHT_CHECK);
            } else if (kingPosition.x == piecePos.x) {
                //vertical
                observation |= 1UL << 31;
                checks.emplace_back(chess::VERTICAL);
            } else if (kingPosition.y == piecePos.y) {
                //horizontal
                observation |= 1UL << 30;
                checks.emplace_back(chess::HORIZONTAL);
            } else { // diagonal checks
                if ((kingPosition.x <= rowMid && kingPosition.y > colMid)
                    || (kingPosition.x > rowMid && kingPosition.y <= colMid)) {
                    //king in upper left or bottom right quadrant
                    if ((piecePos.x > kingPosition.x && piecePos.y < kingPosition.y)
                        || (piecePos.x < kingPosition.x && piecePos.y > kingPosition.y)) {
                        // long diagonal
                        observation |= 1UL << 29;
                        checks.emplace_back(chess::LONG_DIAGONAL);
                    } else if ((piecePos.x < kingPosition.x && piecePos.y < kingPosition.y)
                        || (piecePos.x > kingPosition.x && piecePos.y > kingPosition.y)) {
                        // short diagonal
                        observation |= 1UL << 28;
                        checks.emplace_back(chess::SHORT_DIAGONAL);
                    } else {
                        unreachable("Checking figure is on unknown position!");
                    }
                } else if ((kingPosition.x <= rowMid && kingPosition.y <= colMid)
                    || (kingPosition.x > rowMid && kingPosition.y > colMid)) {
                    //king in upper right or bottom left quadrant
                    if ((piecePos.x < kingPosition.x && piecePos.y < kingPosition.y)
                        || (piecePos.x > kingPosition.x && piecePos.y > kingPosition.y)) {
                        //long diagonal
                        observation |= 1UL << 29;
                        checks.emplace_back(chess::LONG_DIAGONAL);
                    } else if ((piecePos.x < kingPosition.x && piecePos.y > kingPosition.y)
                        || (piecePos.x > kingPosition.x && piecePos.y < kingPosition.y)) {
                        //short diagonal
                        observation |= 1UL << 28;
                        checks.emplace_back(chess::SHORT_DIAGONAL);
                    } else {
                        unreachable("Checking figure is on unknown position!");
                    }
                }
            }
        }

    }
    return make_shared<KriegspielObservation>(observation, checks, capturedP);
}
vector<shared_ptr<Action>> KriegspielState::getAllValidActions(Player player) {
    auto pieces = this->getPiecesOfColor(player);
    vector<shared_ptr<Action>> validActions;
    for (auto piece : pieces) {
        auto pos = piece->getPosition();
        auto va = piece->getAllValidMoves();
        for (auto move : *va) {
            int i = 0;
            validActions.emplace_back(make_shared<KriegspielAction>(i++,
                                                                    pair<shared_ptr<AbstractPiece>,
                                                                         Square>(
                                                                        piece, move),
                                                                    pos));
        }
    }
    return validActions;
}
}

