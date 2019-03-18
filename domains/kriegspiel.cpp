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


#include "kriegspiel.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace chess {
    char coords[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    int bishopMoves[4][2] = {{-1, 1}, {-1, -1}, {1, -1}, {1, 1}};
    int rookMoves[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    int queenKingMoves[8][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}, {-1, 1}, {-1, -1}, {1, -1}, {1, 1}};
    int knightMoves[8][2] = {{1, 2}, {-1, 2}, {-1, -2}, {1, -2}, {2, 1}, {-2, 1}, {-2, -1}, {2, -1}};

    const string coordToString(const Square c) {
        return coords[c.x-1] + std::to_string(c.y);
    }

    const Square stringToCoord(string s) {
        int x = s[0] - 96;
        int y = s[1] - 48;
        return Square(x, y);
    }

    AbstractPiece::AbstractPiece(chess::pieceName k, int c, Square pos, const GTLib2::domains::KriegspielState *s) : position(pos), kind(k), color(c), board(s) {}

    Pawn::Pawn(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b, int id): AbstractPiece(k, c, p, b), id(id) {}
    Rook::Rook(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    Queen::Queen(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    Knight::Knight(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    Bishop::Bishop(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    King::King(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    King::King(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b, bool mo): AbstractPiece(k, c, p, b) {
        this->moved = mo;
    }
    Rook::Rook(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b, bool mo): AbstractPiece(k, c, p, b) {
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
        if(this->kind != PAWN) {
            return this->moved;
        } else {
            if(this->getColor() == WHITE) {
                return this->position.y == 2;
            } else {
                return this->position.y == this->board->getYSize() - 2;
            }
        }
    }

    AbstractPiece* AbstractPiece::getPinnedBy() const {
        return this->pinnedBy;
    }

    AbstractPiece* AbstractPiece::getProtectedBy() const {
        return this->protectedBy;
    }

    void AbstractPiece::setPinnedBy(AbstractPiece* p) {
        this->pinnedBy = p;
    }

    void AbstractPiece::setProtectedBy(AbstractPiece* p) {
        this->protectedBy = p;
    }

    vector<Square>* AbstractPiece::getAllMoves() const {
        return this->moves.get();
    }

    vector<Square>* AbstractPiece::getAllValidMoves() const {
        return this->validMoves.get();
    }

    void AbstractPiece::updateValidMovesWhilePinned() {
        AbstractPiece* pinner = this->getPinnedBy();
        this->validMoves->clear();
        int dx = this->position.x;
        int dy = this->position.y;
        Square pinnerPosition = pinner->getPosition();

        int incrementX = dx == pinnerPosition.x ? 0 : dx < pinnerPosition.x ? 1 : -1;
        int incrementY = dy == pinnerPosition.y ? 0 : dy < pinnerPosition.y ? 1 : -1;

        dx += incrementX;
        dy += incrementY;

        for(; !(dx == pinnerPosition.x && dy == pinnerPosition.y); dx += incrementX, dy += incrementY) {
            Square newPos(dx, dy);
            this->validMoves->push_back(newPos);
        }

        this->validMoves->push_back(pinnerPosition);
    }

    void AbstractPiece::update() {
        this->updateMoves();
        if(this->board->isPlayerInCheck() == this->getColor()) {
            if(this->getPinnedBy() != nullptr) {
                //pinned piece cant block check
                this->validMoves->clear();
                return;
            }
            this->updateValidMovesWhileInCheck();
        } else this->updateValidMovesPinsProtects(this->getPinnedBy() != nullptr);
    }

    void AbstractPiece::updateValidMovesWhileInCheck() {
        if(this->kind == KING) {
            for(int* i: queenKingMoves) {
                Square newPos(this->position.x+i[0], this->position.y+i[1]);
                shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
                if(this->board->coordOutOfBounds(newPos) || this->board->isSquareUnderAttack(invertColor(this->color), newPos) ||
                        (p != nullptr && p->getColor() == this->getColor()) || (p != nullptr && p->getProtectedBy() != nullptr)) {
                    continue;
                }
                this->validMoves->push_back(newPos);

            }
            return;
        }
        if(this->board->getCheckingFigures().size() == 2) {
            this->validMoves->clear();
            return;
        }
        shared_ptr<AbstractPiece> king = this->board->getPiecesOfColorAndKind(this->getColor(), KING)[0];
        shared_ptr<AbstractPiece> checker = this->board->getCheckingFigures().at(0);
        if(checker->getKind() != KNIGHT) {
            vector<Square> blockingSquares = this->board->getSquaresBetween(king.get(), checker.get());
            for(Square s: blockingSquares) {
                if(std::find(this->moves->begin(), this->moves->end(), s) != this->moves->end()) {
                    this->validMoves->push_back(s);
                }
            }
        }
        if(std::find(this->moves->begin(), this->moves->end(), checker->getPosition()) != this->moves->end()) {
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
        return std::string(1 , this->color == chess::WHITE ? toupper(this->kind) : tolower(this->kind));
    }

    bool AbstractPiece::operator==(const chess::AbstractPiece &that) const {
        return this->getPosition() == that.getPosition() && this->getKind() == that.getKind() && this->getColor() == that.getColor();
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

    bool AbstractPiece::equal_to(const AbstractPiece & that) const {
        return this->getColor() == that.getColor() && this->getPosition() == that.getPosition() && this->getKind() == that.getKind();
    }

    void Pawn::updateMoves() {
        //cut moves
        int ycut = this->color == WHITE ? this->position.y + 1 : this->position.y - 1;
        Square cutpos(this->position.x-1, ycut);
        if(this->board->getPieceOnCoords(cutpos) != nullptr) {
            moves->push_back(cutpos);
        }

        if(this->board->getEnPassantSquare() == cutpos) {
            moves->push_back(cutpos);
        }

        Square cutpos2(this->position.x+1, ycut);
        if(this->board->getPieceOnCoords(cutpos2) != nullptr) {
            moves->push_back(cutpos2);
        }

        if(this->board->getEnPassantSquare() == cutpos2) {
            moves->push_back(cutpos2);
        }


        Square moveForward(this->position.x, ycut);
        moves->push_back(moveForward);

        if(!this->moved) {
            Square moveByTwo(this->position.x, this->color == WHITE ? this->position.y + 2 : this->position.y - 2);
            moves->push_back(moveByTwo);
        }
    }

    void Pawn::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        //cut moves
        int ycut = this->color == WHITE ? this->position.y + 1 : this->position.y - 1;
        for(int i = -1; i < 2; i += 2) {
            Square cutpos(this->position.x + i, ycut);
            if(this->board->coordOutOfBounds(cutpos)) continue;
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(cutpos);
            if(p != nullptr && p->getColor() != this->getColor()) {
                if(!onlyPinsAndProtects) validMoves->push_back(cutpos);
            } else if (p != nullptr && p->getColor() == this->getColor()) {
                p->setProtectedBy(this);
            } else if(this->board->getEnPassantSquare() == cutpos) {
                if(!onlyPinsAndProtects) validMoves->push_back(cutpos);
            }
        }

        Square moveForward(this->position.x, ycut);
        if(this->board->getPieceOnCoords(moveForward) == nullptr) {
            if(!onlyPinsAndProtects) validMoves->push_back(moveForward);
        }

        Square moveByTwo(this->position.x, this->color == WHITE ? this->position.y + 2 : this->position.y - 2);
        if(!this->moved && this->board->getPieceOnCoords(moveByTwo) == nullptr && this->board->getPieceOnCoords(moveForward) == nullptr) {
            if(!onlyPinsAndProtects) validMoves->push_back(moveByTwo);
        }
    }

    vector<Square> Pawn::getSquaresAttacked() const {
        vector<Square> v;
        int ycut = this->color == WHITE ? this->position.y + 1 : this->position.y - 1;
        Square pos1(this->position.x-1, ycut);
        v.push_back(pos1);

        Square pos2(this->position.x+1, ycut);
        v.push_back(pos2);
        return v;
    }

    int Pawn::getId() const {
        return this->id;
    }

    void Rook::updateMoves() {
        for(int* i: rookMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!this->board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) continue;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
                if(p != nullptr && p->getColor() == this->getColor()) {
                    breakInner = true;
                    continue;
                }
                this->moves->push_back(newPos);
            }
        }
    }

    void Rook::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: rookMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!this->board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx,dy);
                shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
                if(p != nullptr) {
                    breakInner = true;
                    if(p->getColor() == this->getColor()) {
                        p->setProtectedBy(this);
                        continue;
                    } else {
                        if(this->board->checkPinnedPiece(this, p.get())) {
                            p->setPinnedBy(this);
                            p->updateValidMovesWhilePinned();
                        }
                    }
                }
                if(!onlyPinsAndProtects) this->validMoves->push_back(newPos);
            }
        }
    }

    vector<Square> Rook::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: rookMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!this->board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
                if((p != nullptr && p->getKind() != KING) || (p != nullptr && p->getKind() == KING && p->getColor() == this->getColor())) {
                    breakInner = true;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void Knight::updateMoves() {
        for(int* i: knightMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            Square newPos(dx+i[0], dy+i[1]);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if(!this->board->coordOutOfBounds(newPos) && (p == nullptr || (p != nullptr && p->getColor() != this->getColor()))) {
                this->moves->push_back(newPos);
            }
        }
    }

    void Knight::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: knightMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            Square newPos(dx+i[0], dy+i[1]);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if(!this->board->coordOutOfBounds(newPos)) {
                if(p != nullptr && p->getColor() == this->getColor()) {
                    p->setProtectedBy(this);
                    continue;
                }
                if(!onlyPinsAndProtects) this->validMoves->push_back(newPos);
            }
        }
    }

    vector<Square> Knight::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: knightMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            Square newPos(dx+i[0], dy+i[1]);
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if(!this->board->coordOutOfBounds(newPos)) {
                if(p != nullptr && p->getColor() == this->getColor()) {
                    continue;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void Bishop::updateMoves() {
        for(int* i: bishopMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!this->board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) continue;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
                if(p != nullptr && p->getColor() == this->getColor()) {
                    breakInner = true;
                    continue;
                }
                this->moves->push_back(newPos);
            }
        }
    }

    void Bishop::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: bishopMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!this->board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
                if(p != nullptr) {
                    breakInner = true;
                    if(p->getColor() == this->getColor()) {
                        p->setProtectedBy(this);
                        continue;
                    } else {
                        if(this->board->checkPinnedPiece(this, p.get())) {
                            p->setPinnedBy(this);
                            p->updateValidMovesWhilePinned();
                        }
                    }
                }
                if(!onlyPinsAndProtects) this->validMoves->push_back(newPos);
            }
        }
    }

    vector<Square> Bishop::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: bishopMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!this->board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
                if((p != nullptr && p->getKind() != KING) || (p != nullptr && p->getKind() == KING && p->getColor() == this->getColor())) {
                    breakInner = true;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void Queen::updateMoves() {
        for(int* i: queenKingMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!this->board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) continue;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
                if(p != nullptr && p->getColor() == this->getColor()) {
                    breakInner = true;
                    continue;
                }
                this->moves->push_back(newPos);
            }
        }
    }

    void Queen::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: queenKingMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!this->board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
                if(p != nullptr) {
                    breakInner = true;
                    if(p->getColor() == this->getColor()) {
                        p->setProtectedBy(this);
                        continue;
                    } else {
                        if(this->board->checkPinnedPiece(this, p.get())) {
                            p->setPinnedBy(this);
                            p->updateValidMovesWhilePinned();
                        }
                    }
                }
                if(!onlyPinsAndProtects) this->validMoves->push_back(newPos);
            }
        }
    }

    vector<Square> Queen::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: queenKingMoves) {
            int dx = this->position.x;
            int dy = this->position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!this->board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
                if((p != nullptr && p->getKind() != KING) || (p != nullptr && p->getKind() == KING && p->getColor() == this->getColor())) {
                    breakInner = true;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void King::updateMoves() {
        for(int* i: queenKingMoves) {
            Square newPos(this->position.x+i[0], this->position.y+i[1]);
            if(this->board->coordOutOfBounds(newPos)) {
                continue;
            }
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if(p != nullptr && p->getColor() == this->getColor()) continue;
            this->moves->push_back(newPos);
        }

        if(!this->hasMoved() && this->board->isPlayerInCheck() != this->getColor() && this->board->canCastle()) {
            vector<shared_ptr<AbstractPiece>> rooks = this->board->getPiecesOfColorAndKind(this->color, ROOK);
            for(const shared_ptr<AbstractPiece>& r: rooks) {
                if(!r->hasMoved()) {
                    vector<Square> squaresBetween = this->board->getSquaresBetween(r.get(), this);
                    if(squaresBetween.size() == 2) {
                        //short castle

                        bool breakinner = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(this->board->getPieceOnCoords(i) != nullptr) {
                                breakinner = true;
                                break;
                            }
                        }
                        if(breakinner) continue;

                        int dx = r->getPosition().x > this->position.x ? r->getPosition().x - 1: r->getPosition().x + 1;
                        Square newPos(dx, this->position.y);
                        this->moves->push_back(newPos);
                    } else {
                        //long castle

                        bool breakinner = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(this->board->getPieceOnCoords(i) != nullptr) {
                                breakinner = true;
                                break;
                            }
                        }
                        if(breakinner) continue;


                        int dx = r->getPosition().x > this->position.x ? r->getPosition().x - 2: r->getPosition().x + 2;
                        Square newPos(dx, this->position.y);
                        this->moves->push_back(newPos);
                    }
                }
            }
        }
    }

    void King::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: queenKingMoves) {
            Square newPos(this->position.x+i[0], this->position.y+i[1]);
            if(this->board->coordOutOfBounds(newPos) || this->board->isSquareUnderAttack(invertColor(this->getColor()), newPos)) {
                continue;
            }
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if(p != nullptr && p->getColor() == this->getColor()) {
                p->setProtectedBy(this);
                continue;
            }
            if(p != nullptr && p->getProtectedBy() != nullptr) continue;
            this->validMoves->push_back(newPos);
        }

        if(!this->hasMoved() && this->board->isPlayerInCheck() != this->getColor() && this->board->canCastle()) {
            vector<shared_ptr<AbstractPiece>> rooks = this->board->getPiecesOfColorAndKind(this->color, ROOK);
            for(const shared_ptr<AbstractPiece>& r: rooks) {
                if(!r->hasMoved()) {
                    vector<Square> squares = this->board->getSquaresBetween(r.get(), this);
                    bool breakinner = false;
                    for(Square i: squares) {
                        if(this->board->isSquareUnderAttack(invertColor(this->color), i)) {
                            breakinner = true;
                        }
                    }
                    if(breakinner) continue;
                    vector<Square> squaresBetween = this->board->getSquaresBetween(r.get(), this);
                    if(squaresBetween.size() == 2) {
                        //short castle

                        bool breakin = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(this->board->getPieceOnCoords(i) != nullptr) {
                                breakin = true;
                                break;
                            }
                        }
                        if(breakin) continue;

                        int dx = r->getPosition().x > this->position.x ? r->getPosition().x - 1: r->getPosition().x + 1;
                        Square newPos(dx, this->position.y);
                        this->validMoves->push_back(newPos);
                    } else {
                        //long castle

                        bool breakin = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(this->board->getPieceOnCoords(i) != nullptr) {
                                breakin = true;
                                break;
                            }
                        }
                        if(breakin) continue;

                        int dx = r->getPosition().x > this->position.x ? r->getPosition().x - 2: r->getPosition().x + 2;
                        Square newPos(dx, this->position.y);
                        this->validMoves->push_back(newPos);
                    }
                }
            }
        }
    }

    vector<Square> King::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: queenKingMoves) {
            Square newPos(this->position.x+i[0], this->position.y+i[1]);
            if(this->board->coordOutOfBounds(newPos)) {
                continue;
            }
            shared_ptr<AbstractPiece> p = this->board->getPieceOnCoords(newPos);
            if(p != nullptr) {
                continue;
            }
            v.push_back(newPos);
        }
        return v;
    }
}

namespace GTLib2 {
    namespace domains {
        using chess::Square;

        KriegspielAction::KriegspielAction(int id, pair<shared_ptr<AbstractPiece>, Square> move, Square originalPos): Action(id), moveFrom(originalPos), move_(std::move(move)) {
        }

        KriegspielAction::KriegspielAction(int id): Action(id), moveFrom(-1, -1), move_(pair<shared_ptr<AbstractPiece>, Square>(nullptr, Square(-1, -1))) {

        }

        Square KriegspielAction::movingFrom() const {
            return this->moveFrom;
        }

        KriegspielDomain::KriegspielDomain(unsigned int maxDepth, unsigned int legalMaxDepth, chess::BOARD b):Domain(maxDepth, 2) {
            vector<double> rewards(2);
            vector<shared_ptr<Observation>> Obs{make_shared<Observation>(-1), make_shared<Observation>(-1)};
            Outcome o(make_shared<KriegspielState>(this, legalMaxDepth, b), Obs, rewards);
            rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
            maxUtility = 1;
        }
        KriegspielObservation::KriegspielObservation(int id):Observation(id) {}
        KriegspielState::KriegspielState(GTLib2::Domain *domain, int legalMaxDepth, chess::BOARD b):State(domain), enPassantSquare(-1, -1) {
            this->moveHistory = make_shared<vector<shared_ptr<KriegspielAction>>>();
            this->attemptedMoveHistory = make_shared<vector<shared_ptr<KriegspielAction>>>();
            this->playerOnTheMove = chess::WHITE;
            this->legalMaxDepth = legalMaxDepth;
            this->initBoard(b);
            this->updateState(chess::WHITE);
        }

        KriegspielState::KriegspielState(Domain *domain, int legalMaxDepth, string s): State(domain), enPassantSquare(-1, -1) {
            this->moveHistory = make_shared<vector<shared_ptr<KriegspielAction>>>();
            this->attemptedMoveHistory = make_shared<vector<shared_ptr<KriegspielAction>>>();
            this->playerOnTheMove = chess::WHITE;
            this->legalMaxDepth = legalMaxDepth;
            this->initBoard(std::move(s));
            this->updateAllPieces();
        }

        KriegspielState::KriegspielState(GTLib2::Domain *domain, int legalMaxDepth, int x, int y, shared_ptr<vector<shared_ptr<chess::AbstractPiece>>> pieces,
                                         Square enPassantSquare, shared_ptr<vector<shared_ptr<KriegspielAction>>> moves, int p, bool castle,
                                         shared_ptr<vector<shared_ptr<KriegspielAction>>> attemptedMoves):State(domain), enPassantSquare(enPassantSquare) {
            this->xSize = x;
            this->ySize = y;
            this->pieces = std::move(pieces);
            this->moveHistory = std::move(moves);
            this->legalMaxDepth = legalMaxDepth;
            this->attemptedMoveHistory = std::move(attemptedMoves);
            this->canPlayerCastle = castle;

            //rebind to new board state
            for(shared_ptr<AbstractPiece>& piece: *this->pieces) {
                piece->setBoard(this);
            }
            this->playerOnTheMove = p;
        }

        pair<shared_ptr<AbstractPiece>, Square> KriegspielAction::getMove() const {
            return this->move_;
        }

        bool KriegspielAction::operator==(const GTLib2::Action &that) const {
            const auto rhsAction = dynamic_cast<const KriegspielAction *>(&that);
            return this->move_.first->equal_to(*rhsAction->move_.first) && this->move_.second == rhsAction->move_.second && this->movingFrom() == rhsAction->movingFrom();
        }

        size_t KriegspielAction::getHash() const {
            std::hash<string> h;
            string s = this->move_.first->toString() + chess::coordToString(this->move_.second) + chess::coordToString(this->movingFrom());
            return h(s);
        }

        shared_ptr<KriegspielAction> KriegspielAction::clone() const {
            Square s = Square(this->getMove().second.x, this->getMove().second.y);
            shared_ptr<AbstractPiece> p = this->getMove().first->clone();
            Square f = Square(this->movingFrom().x, this->movingFrom().y);
            auto a = make_shared<KriegspielAction>(this->id, pair<shared_ptr<AbstractPiece>, chess::Square>(p, s), f);
            return a;
        }

        string KriegspielDomain::getInfo() const {
            return "************ Kriegspiel *************\n" +
                   rootStatesDistribution[0].first.state->toString() + "\n";
        }

        KriegspielDomain::KriegspielDomain(unsigned int maxDepth, unsigned int legalMaxDepth, string s): Domain(maxDepth, 2) {
            vector<double> rewards(2);
            vector<shared_ptr<Observation>> Obs{make_shared<Observation>(-1), make_shared<Observation>(-1)};
            Outcome o(make_shared<KriegspielState>(this, legalMaxDepth, s), Obs, rewards);
            rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
        }

        vector<shared_ptr<Action>> KriegspielState::getAvailableActionsFor(int player) const {
            auto list = vector<shared_ptr<Action>>();
            if(player != this->playerOnTheMove || this->gameHasEnded || this->moveHistory->size() == this->legalMaxDepth) {
                return list;
            }
            int count = 0;
            for (shared_ptr<AbstractPiece>p: *this->pieces) {
                if(p->getColor() != player) continue;
                auto moves = p->getAllMoves();
                for(Square move: *moves) {
                    auto ksa = make_shared<KriegspielAction>(count, std::pair<shared_ptr<AbstractPiece>, Square>(p, move), p->getPosition());
                    if(std::find(attemptedMoveHistory->begin(), attemptedMoveHistory->end(), ksa) == attemptedMoveHistory->end()) {
                        list.push_back(ksa);
                        ++count;
                    }
                }
            }
            return list;
        }

        void KriegspielState::promote(shared_ptr<AbstractPiece> p, Square pos) const {
            auto q = make_shared<chess::Queen>(chess::QUEEN, p->getColor(), pos, this);
            for(auto It = this->pieces->begin(); It != this->pieces->end();) {
                if(*(*It) == *p) {
                    this->pieces->erase(It);
                    this->pieces->push_back(q);
                    break;
                }
                else {
                    ++It;
                }
            }
        }

        vector<shared_ptr<AbstractPiece>> KriegspielState::getPiecesOfColor(int color) const {
            vector<shared_ptr<AbstractPiece>> v;
            for(shared_ptr<AbstractPiece>& p: *this->pieces) {
                if(p->getColor() == color) {
                    v.push_back(p);
                }
            }
            return v;
        }

        int KriegspielState::isPlayerInCheck() const {
            return this->playerInCheck;
        }

        vector<shared_ptr<AbstractPiece>> KriegspielState::getCheckingFigures() const {
            return this->checkingFigures;
        }

        int KriegspielState::checkGameOverCheck(int c) const {
            int inCheck = -1;
            shared_ptr<AbstractPiece> king = this->getPiecesOfColorAndKind(c, chess::KING)[0];
            for(const shared_ptr<AbstractPiece>& p: *this->pieces) {
                if(p->getColor() == c) continue;
                Square kingPos = king->getPosition();
                vector<Square> attackSquares = p->getSquaresAttacked();
                if(std::find(attackSquares.begin(), attackSquares.end(), kingPos) != attackSquares.end()) {
                    inCheck = king->getColor();
                }
            }
            return inCheck;
        }

        void KriegspielState::checkPlayerInCheck() {
            int kingColor = this->playerOnTheMove;
            this->playerInCheck = -1;
            shared_ptr<AbstractPiece> king = this->getPiecesOfColorAndKind(kingColor, chess::KING)[0];
            for(const shared_ptr<AbstractPiece>& p: *this->pieces) {
                if(p->getColor() == kingColor) continue;
                Square kingPos = king->getPosition();
                vector<Square> attackSquares = p->getSquaresAttacked();
                if(std::find(attackSquares.begin(), attackSquares.end(), kingPos) != attackSquares.end()) {
                    this->playerInCheck = kingColor;
                    this->checkingFigures.push_back(p);
                }
            }
        }

        bool KriegspielState::canCastle() const {
            return this->canPlayerCastle;
        }

        void KriegspielState::castle(KriegspielAction* a) const {
            pair<shared_ptr<AbstractPiece>, Square> move = a->getMove();
            shared_ptr<AbstractPiece> king = move.first;
            Square pos = move.second;
            int dx = king->getPosition().x;
            int dy = king->getPosition().y;
            //get rook in the direction of the move so we don't have to make special cases for other boards and black/white board asymmetry
            int incrementX = pos.x > dx ? 1 : -1;
            shared_ptr<AbstractPiece> rook;
            dx += incrementX;
            for(; ((rook = this->getPieceOnCoords(Square(dx, dy))) == nullptr); dx += incrementX);
            int newRookX = pos.x > rook->getPosition().x ? pos.x + 1 : pos.x - 1;
            Square newRookPos(newRookX, rook->getPosition().y);
            rook->move(newRookPos);
        }

        bool KriegspielState::move(KriegspielAction * a) {
            pair<shared_ptr<AbstractPiece>, Square> move = a->getMove();
            shared_ptr<AbstractPiece> p = this->getPieceOnCoords(move.first->getPosition());
            Square pos = move.second;

            if(std::find(p->getAllValidMoves()->begin(), p->getAllValidMoves()->end(), pos) != p->getAllValidMoves()->end()) {
                //move valid
                shared_ptr<AbstractPiece> checkCut = this->getPieceOnCoords(pos);
                if(checkCut != nullptr) {
                    lastCut = 30 + checkCut->getPosition().x + ((checkCut->getPosition().y-1) * this->ySize);
                    for(auto It = this->pieces->begin(); It != this->pieces->end();) {
                        if(*(*It) == *checkCut) {
                            this->pieces->erase(It);
                            break;
                        }
                        else {
                            ++It;
                        }
                    }
                }

                if(pos.x == this->enPassantSquare.x && pos.y == this->enPassantSquare.y) {
                    int pawnColor = chess::invertColor(p->getColor());
                    int y = pawnColor == chess::WHITE ? pos.y + 1 : pos.y - 1;
                    Square enPassPos(pos.x, y);
                    shared_ptr<AbstractPiece> enPassPawn = this->getPieceOnCoords(enPassPos);
                    if(enPassPawn != nullptr) {
                        for(auto It = this->pieces->begin(); It != this->pieces->end();) {
                            if(*(*It) == *enPassPawn) {
                                this->pieces->erase(It);
                                break;
                            }
                            else {
                                ++It;
                            }
                        }
                    }
                }

                if(p->getKind() == chess::KING && abs(a->movingFrom().x - pos.x) > 1) {
                    this->castle(a);
                }
                p->move(pos);
                this->checkPlayerInCheck();
                if(p->getKind() == chess::PAWN && ((pos.y == this->getYSize() && p->getColor() == chess::WHITE) || (pos.y == 1 && p->getColor() == chess::BLACK))) {
                    this->promote(p, pos);
                }
                return true;
            } else {
                return false;
            }
        }

        vector<double> KriegspielState::checkGameOver() const {
            vector<double> rewards(2);
            vector<shared_ptr<AbstractPiece>> whitePieces = this->getPiecesOfColor(0);
            vector<shared_ptr<AbstractPiece>> blackPieces = this->getPiecesOfColor(1);
            if(whitePieces.size() <= 2 && blackPieces.size() <= 2) {
                bool isDraw = true;
                whitePieces.insert(whitePieces.end(), blackPieces.begin(), blackPieces.end());
                for(const shared_ptr<AbstractPiece>& p: whitePieces) {
                    if(p->getKind() == chess::KING) continue;
                    else if(p->getKind() == chess::PAWN || p->getKind() == chess::ROOK || p->getKind() == chess::QUEEN) isDraw = false;
                }
                if(isDraw) {
                    rewards[chess::WHITE] = 0.5;
                    rewards[chess::BLACK] = 0.5;
                    return rewards;
                }
            }
            rewards[chess::WHITE] = 0;
            rewards[chess::BLACK] = 0;
            bool hasMoves[] = {false, false};
            int isCheck = this->checkGameOverCheck(this->playerOnTheMove);

            for(const shared_ptr<AbstractPiece>& p: *this->pieces) {
                hasMoves[p->getColor()] = hasMoves[p->getColor()] ? true : !p->getAllValidMoves()->empty();
            }

            if(isCheck == this->playerOnTheMove) {
                if(!hasMoves[this->playerOnTheMove]) {
                    rewards[this->playerOnTheMove] = 0;
                    rewards[chess::invertColor(this->playerOnTheMove)] = 1;
                } else if(this->moveHistory->size() == this->legalMaxDepth) {
                    rewards[chess::WHITE] = 0.5;
                    rewards[chess::BLACK] = 0.5;
                } else if(this->moveHistory->size() + this->attemptedMoveHistory->size() == this->domain->getMaxDepth()) {
                    rewards[chess::WHITE] = 0.5;
                    rewards[chess::BLACK] = 0.5;
                }
            } else if(!hasMoves[this->playerOnTheMove]) {
                rewards[chess::WHITE] = 0.5;
                rewards[chess::BLACK] = 0.5;
            } else if(this->moveHistory->size() == this->legalMaxDepth) {
                rewards[chess::WHITE] = 0.5;
                rewards[chess::BLACK] = 0.5;
            }

            return rewards;
        }

        Square KriegspielState::getEnPassantSquare() const {
            return this->enPassantSquare;
        }



        OutcomeDistribution KriegspielState::performActions(
                const vector<pair<int, shared_ptr<GTLib2::Action>>> &actions) const {
            auto a1 = dynamic_cast<KriegspielAction*>(actions[0].second.get());
            auto a2 = dynamic_cast<KriegspielAction*>(actions[1].second.get());
            vector<shared_ptr<Observation>> observations(2);
            vector<double> rewards(2);
            shared_ptr<KriegspielState> s;
            int nextMove = this->playerOnTheMove;
            Square enPassSquare(-1, -1);
            KriegspielAction* a = a1 ? a1 : a2;
            shared_ptr<vector<shared_ptr<AbstractPiece>>> pieces = this->copyPieces();
            shared_ptr<vector<shared_ptr<KriegspielAction>>> history = this->copyMoveHistory();
            shared_ptr<vector<shared_ptr<KriegspielAction>>> attemptedmoves = this->copyAttemptedMoves();
            s = make_shared<KriegspielState>(domain, this->legalMaxDepth, this->xSize, this->ySize, pieces, this->enPassantSquare, history, nextMove, this->canPlayerCastle, attemptedmoves);
            s->updateState(this->playerOnTheMove);
            shared_ptr<KriegspielAction> ac = a->clone();
            if(s->move(a)) {
                s->addToHistory(ac);
                enPassSquare = this->checkEnPassant(a);
                nextMove = chess::invertColor(this->playerOnTheMove);
                s->setPlayerOnMove(nextMove);
                s->setEnPassant(enPassSquare);
                s->updateState(nextMove);
                rewards = s->checkGameOver();
                observations[this->playerOnTheMove] = make_shared<Observation>(1);
                observations[chess::invertColor(this->playerOnTheMove)] = make_shared<Observation>(s->calculateObservation(chess::invertColor(this->playerOnTheMove)));
                int x = 4;
            } else {
                s->addToAttemptedMoves(ac);
                nextMove = this->playerOnTheMove;
                observations[this->playerOnTheMove] = make_shared<Observation>(a->getId());
                observations[chess::invertColor(this->playerOnTheMove)] = make_shared<Observation>(-1);
                s->updateState(nextMove);
                rewards = s->checkGameOver();
            }
            if(rewards[0] != 0 || rewards[1] != 0) {
                s->setGameHasEnded(true);
            }
            Outcome o(std::move(s), observations, rewards);
            OutcomeDistribution prob;
            prob.push_back(std::pair<Outcome, double>(std::move(o), 1.0));
            return prob;
        }

        Square KriegspielState::checkEnPassant(KriegspielAction * a) const {
            shared_ptr<AbstractPiece> abstractPiece = a->getMove().first;
            Square pos(-1, -1);
            if(abstractPiece->getKind() != chess::PAWN) {
                return pos;
            } else {
                if((a->getMove().second.y - a->movingFrom().y) == 2) {
                    pos.x = a->movingFrom().x;
                    pos.y = a->movingFrom().y + 1;
                    return pos;
                } else if ((a->getMove().second.y - a->movingFrom().y) == -2){
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
            if(this->xSize != rhsState->xSize || this->ySize != rhsState->ySize) {
                return false;
            }
            for(int i = 1; i <= this->xSize; i++) {
                for(int j = 1; j <= this->ySize; j++) {
                    shared_ptr<AbstractPiece> p1 = this->getPieceOnCoords(Square(i, j));
                    shared_ptr<AbstractPiece> p2 = this->getPieceOnCoords(Square(i, j));
                    if(p1 == nullptr && p2 == nullptr) continue;
                    if((p1 != nullptr && p2 == nullptr) || (p1 == nullptr && p2 != nullptr)) return false;
                    if(!(*p1 == *p2)) return false;
                }
            }
            return true;
        }

        size_t KriegspielState::getHash() const {
            std::hash<string> h;
            return h(this->toString());
        }

        string KriegspielState::toString() const {
            //DEPRECATED
            vector<vector<string>> board;
            for(int j = 0; j < this->ySize; j++) {
                vector<string> row;
                for(int i = 0; i < this->xSize; i++) {
                    row.emplace_back("_");
                }
                board.push_back(row);
            }

            for(shared_ptr<AbstractPiece>p: *this->pieces) {
                Square pos = p->getPosition();
                pos.y -= 1;
                pos.x -= 1;
                board.at(pos.y).at(pos.x) = p->toString();
            }

            string s;
            for(int i = this->ySize-1; i >= 0; i--) {
                vector<string> row = board.at(i);
                for(const string &str: row) {
                    s += str;
                }
                s += "\n";
            }
            s += "oooooooo\n";
            return s;

            /*chess::FenBoardFactory f;
            return f.createFEN(this);*/
        }

        shared_ptr<AbstractPiece> KriegspielState::getPieceOnCoords(Square pos) const {
            for(const shared_ptr<AbstractPiece>& p: *this->pieces) {
                if(p->getPosition() == pos) {
                    return p;
                }
            }
            return nullptr;
        }

        bool KriegspielState::coordOutOfBounds(Square pos) const {
            return pos.x > this->xSize || pos.y > this->ySize || pos.x < 1 || pos.y < 1;
        }

        void KriegspielState::updateAllPieces() const {
            for(shared_ptr<AbstractPiece>& p: *this->pieces) {
                p->update();
            }
        }

        void KriegspielState::resetAllPieces() {
            for(shared_ptr<AbstractPiece>& p: *this->pieces) {
                p->reset();
            }
        }

        void KriegspielState::updatePiecesOfColor(int color) const {
            vector<shared_ptr<AbstractPiece>> v = this->getPiecesOfColor(color);
            for(shared_ptr<AbstractPiece>& p: v) {
                p->update();
            }
        }

        vector<shared_ptr<AbstractPiece>> KriegspielState::getPiecesOfColorAndKind(int c, chess::pieceName n) const {
            vector<shared_ptr<AbstractPiece>> v;
            for(const shared_ptr<AbstractPiece>& p: *this->pieces) {
                if(p->getColor() == c && p->getKind() == n) {
                    v.push_back(p);
                }
            }
            return v;
        }

        bool KriegspielState::isSquareUnderAttack(int color, Square square) const {
            vector<shared_ptr<AbstractPiece>> pieces = this->getPiecesOfColor(color);
            for(const shared_ptr<AbstractPiece>& p: pieces) {
                vector<Square> attackedSquares = p->getSquaresAttacked();
                if(std::find(attackedSquares.begin(), attackedSquares.end(), square) != attackedSquares.end()) {
                    return true;
                }
            }
            return false;
        }

        void KriegspielState::clearBoard() {
            this->pieces->clear();
        }

        void KriegspielState::insertPiece(shared_ptr<AbstractPiece> p) {
            this->pieces->push_back(p);
        }

        vector<Square> KriegspielState::getSquaresPierced(AbstractPiece* pierced, AbstractPiece* piercer) const {
            Square posPiercer = piercer->getPosition();
            Square posPierced = pierced->getPosition();
            vector<Square> squares;

            int dx = posPierced.x;
            int dy = posPierced.y;

            int incrementX = dx == posPiercer.x ? 0 : dx < posPiercer.x ? -1 : 1;
            int incrementY = dy == posPiercer.y ? 0 : dy < posPiercer.y ? -1 : 1;

            dx += incrementX;
            dy += incrementY;

            for(; !this->coordOutOfBounds(Square(dx, dy)); dx += incrementX, dy += incrementY) {
                Square newPos(dx, dy);
                squares.push_back(newPos);
            }
            return squares;
        }

        vector<Square> KriegspielState::getSquaresBetween(AbstractPiece* p1, AbstractPiece* p2) const {

            Square pos1 = p1->getPosition();
            Square pos2 = p2->getPosition();
            vector<Square> squares;

            int dx = pos1.x;
            int dy = pos1.y;

            int incrementX = dx == pos2.x ? 0 : dx < pos2.x ? 1 : -1;
            int incrementY = dy == pos2.y ? 0 : dy < pos2.y ? 1 : -1;

            dx += incrementX;
            dy += incrementY;

            for(; !(dx == pos2.x && dy == pos2.y); dx += incrementX, dy += incrementY) {
                Square newPos(dx, dy);
                if(this->coordOutOfBounds(newPos)) {
                    string s1 = "Position 1: " + to_string(p1->getPosition().x) + ", " + to_string(p1->getPosition().y) + "\r\n"
                                                                                                                          "Figure 1: " + p1->toString() + "\r\n";
                    string s2 = "Position 2: " + to_string(p2->getPosition().x) + ", " + to_string(p2->getPosition().y) + "\r\n"
                                                                                                                          "Figure 2: " + p2->toString() + "\r\n";
                    std::cout << s1;
                    std::cout << s2;
                    throw std::invalid_argument("wrong figures");
                }
                squares.push_back(newPos);
            }

            return squares;
        }

        bool KriegspielState::checkPinnedPiece(AbstractPiece* pinner, AbstractPiece* pinned) const {
            int kingColor = pinned->getColor();
            shared_ptr<AbstractPiece> king = this->getPiecesOfColorAndKind(kingColor, chess::pieceName::KING)[0];
            vector<Square> squaresToCheck = this->getSquaresPierced(pinned, pinner);
            for(Square i: squaresToCheck) {
                shared_ptr<AbstractPiece> p = this->getPieceOnCoords(i);
                if(p != nullptr) {
                    return p == king;
                }
            }
            return false;
        }

        void KriegspielState::initBoard(chess::BOARD b) {
            chess::BoardFactory bf;
            chess::boardInfo binfo = bf.create(b, this);
            this->pieces = binfo.pieces;
            this->xSize = binfo.x;
            this->ySize = binfo.y;
            this->canPlayerCastle = binfo.castle;
        }

        int KriegspielState::getXSize() const {
            return this->xSize;
        }

        int KriegspielState::getYSize() const {
            return this->ySize;
        }

        void KriegspielState::setPlayerOnMove(int p) {
            this->playerOnTheMove = p;
        }

        void KriegspielState::setGameHasEnded(bool gameHasEnded) {
            this->gameHasEnded = gameHasEnded;
        }

        shared_ptr<vector<shared_ptr<AbstractPiece>>> KriegspielState::copyPieces() const {
            shared_ptr<vector<shared_ptr<AbstractPiece>>> v = make_shared<vector<shared_ptr<AbstractPiece>>>();
            for(const shared_ptr<AbstractPiece>& p: *this->pieces) {
                auto asdf = p->clone();
                v->push_back(asdf);
            }
            return v;
        }

        shared_ptr<vector<shared_ptr<KriegspielAction>>> KriegspielState::copyMoveHistory() const {
            shared_ptr<vector<shared_ptr<KriegspielAction>>> v = make_shared<vector<shared_ptr<KriegspielAction>>>();
            for(const shared_ptr<KriegspielAction>& a: *this->moveHistory) {
                auto asdf = a->clone();
                v->push_back(asdf);
            }
            return v;
        }

        void KriegspielState::addToHistory(shared_ptr<KriegspielAction> a) {
            this->moveHistory->push_back(a);
            this->attemptedMoveHistory->clear();
        }

        void KriegspielState::setEnPassant(chess::Square s) {
            this->enPassantSquare = s;
            //refresh moves on pawns after setting enpassant square
            vector<shared_ptr<AbstractPiece>> pawns = this->getPiecesOfColorAndKind(this->playerOnTheMove, chess::PAWN);
            for(shared_ptr<AbstractPiece>& p: pawns) {
                p->update();
            }
        }

        void KriegspielState::updateState(int p) {
            this->playerOnTheMove = p;
            this->resetAllPieces();
            this->updatePiecesOfColor(chess::invertColor(this->playerOnTheMove));
            this->checkPlayerInCheck();
            this->updatePiecesOfColor(this->playerOnTheMove);

            //update kings again to remove moves that would cut protected pieces
            shared_ptr<AbstractPiece> whiteKing = this->getPiecesOfColorAndKind(chess::WHITE, chess::KING)[0];
            shared_ptr<AbstractPiece> blackKing = this->getPiecesOfColorAndKind(chess::BLACK, chess::KING)[0];
            whiteKing->reset();
            blackKing->reset();
            whiteKing->update();
            blackKing->update();
        }

        void KriegspielState::initBoard(string s) {
            chess::FenBoardFactory bf;
            chess::boardInfo binfo = bf.create(std::move(s), this);
            this->pieces = binfo.pieces;
            this->xSize = binfo.x;
            this->ySize = binfo.y;
            this->canPlayerCastle = binfo.castle;
            if(binfo.xEnpass) {
                Square sq(binfo.xEnpass, binfo.yEnpass);
                this->setEnPassant(sq);
            }
        }

        shared_ptr<vector<shared_ptr<KriegspielAction>>> KriegspielState::copyAttemptedMoves() const {
            shared_ptr<vector<shared_ptr<KriegspielAction>>> v = make_shared<vector<shared_ptr<KriegspielAction>>>();
            for(const shared_ptr<KriegspielAction>& a: *this->attemptedMoveHistory) {
                auto asdf = a->clone();
                v->push_back(asdf);
            }
            return v;
        }

        void KriegspielState::addToAttemptedMoves(shared_ptr<KriegspielAction> a) {
            this->attemptedMoveHistory->push_back(a);
        }

        int KriegspielState::calculateObservation(int player) const {
            auto pawns = getPiecesOfColorAndKind(player, chess::PAWN);
            int toreturn = lastCut;
            for(const shared_ptr<AbstractPiece>& p: pawns) {
                auto curr = dynamic_cast<chess::Pawn*>(p.get());
                for(auto move: *curr->getAllMoves()) {
                    if(move.x == curr->getPosition().x - 1) {
                        toreturn += curr->getId();
                    } else if(move.x == curr->getPosition().x + 1) {
                        toreturn += curr->getId()+1;
                    }
                }
            }
            return toreturn;
        }
    }
}

#pragma clang diagnostic pop
