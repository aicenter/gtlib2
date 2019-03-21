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

    AbstractPiece::AbstractPiece(chess::pieceName k, int c, Square pos, const GTLib2::domains::KriegspielState *s) : position_(pos), kind_(k), color_(c), board_(s) {}

    Pawn::Pawn(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b, int id): AbstractPiece(k, c, p, b), id_(id) {}
    Rook::Rook(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    Queen::Queen(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    Knight::Knight(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    Bishop::Bishop(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    King::King(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    King::King(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b, bool mo): AbstractPiece(k, c, p, b) {
        moved_ = mo;
    }
    Rook::Rook(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b, bool mo): AbstractPiece(k, c, p, b) {
        moved_ = mo;
    }

    shared_ptr<AbstractPiece> Queen::clone() const {
        return make_shared<Queen>(kind_, color_, position_, board_);
    }

    shared_ptr<AbstractPiece> Pawn::clone() const {
        return make_shared<Pawn>(kind_, color_, position_, board_, id_);
    }

    shared_ptr<AbstractPiece> Knight::clone() const {
        return make_shared<Knight>(kind_, color_, position_, board_);
    }

    shared_ptr<AbstractPiece> King::clone() const {
        return make_shared<King>(kind_, color_, position_, board_, moved_);
    }

    shared_ptr<AbstractPiece> Rook::clone() const {
        return make_shared<Rook>(kind_, color_, position_, board_, moved_);
    }

    shared_ptr<AbstractPiece> Bishop::clone() const {
        return make_shared<Bishop>(kind_, color_, position_, board_);
    }

    int invertColor(int c) {
        return c == WHITE ? BLACK : WHITE;
    }

    void AbstractPiece::move(Square pos) {
        position_ = pos;
        moved_ = true;
    }

    bool AbstractPiece::hasMoved() const {
        if(kind_ != PAWN) {
            return moved_;
        } else {
            if(getColor() == WHITE) {
                return position_.y == 2;
            } else {
                return position_.y == board_->getYSize() - 2;
            }
        }
    }

    AbstractPiece* AbstractPiece::getPinnedBy() const {
        return pinnedBy_;
    }

    AbstractPiece* AbstractPiece::getProtectedBy() const {
        return protectedBy_;
    }

    void AbstractPiece::setPinnedBy(AbstractPiece* p) {
        pinnedBy_ = p;
    }

    void AbstractPiece::setProtectedBy(AbstractPiece* p) {
        protectedBy_ = p;
    }

    vector<Square>* AbstractPiece::getAllMoves() const {
        return moves_.get();
    }

    vector<Square>* AbstractPiece::getAllValidMoves() const {
        return validMoves_.get();
    }

    void AbstractPiece::updateValidMovesWhilePinned() {
        AbstractPiece* pinner = getPinnedBy();
        validMoves_->clear();
        int dx = position_.x;
        int dy = position_.y;
        Square pinnerPosition = pinner->getPosition();

        int incrementX = dx == pinnerPosition.x ? 0 : dx < pinnerPosition.x ? 1 : -1;
        int incrementY = dy == pinnerPosition.y ? 0 : dy < pinnerPosition.y ? 1 : -1;

        dx += incrementX;
        dy += incrementY;

        for(; !(dx == pinnerPosition.x && dy == pinnerPosition.y); dx += incrementX, dy += incrementY) {
            Square newPos(dx, dy);
            validMoves_->push_back(newPos);
        }

        validMoves_->push_back(pinnerPosition);
    }

    void AbstractPiece::update() {
        updateMoves();
        if(board_->isPlayerInCheck() == getColor()) {
            if(getPinnedBy() != nullptr) {
                //pinned piece cant block check
                validMoves_->clear();
                return;
            }
            updateValidMovesWhileInCheck();
        } else updateValidMovesPinsProtects(getPinnedBy() != nullptr);
    }

    void AbstractPiece::updateValidMovesWhileInCheck() {
        if(kind_ == KING) {
            for(int* i: queenKingMoves) {
                Square newPos(position_.x+i[0], position_.y+i[1]);
                shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
                if(board_->coordOutOfBounds(newPos) || board_->isSquareUnderAttack(invertColor(color_), newPos) ||
                        (p != nullptr && p->getColor() == getColor()) || (p != nullptr && p->getProtectedBy() != nullptr)) {
                    continue;
                }
                validMoves_->push_back(newPos);

            }
            return;
        }
        if(board_->getCheckingFigures().size() == 2) {
            validMoves_->clear();
            return;
        }
        shared_ptr<AbstractPiece> king = board_->getPiecesOfColorAndKind(getColor(), KING)[0];
        shared_ptr<AbstractPiece> checker = board_->getCheckingFigures().at(0);
        if(checker->getKind() != KNIGHT) {
            vector<Square> blockingSquares = board_->getSquaresBetween(king.get(), checker.get());
            for(Square s: blockingSquares) {
                if(std::find(moves_->begin(), moves_->end(), s) != moves_->end()) {
                    validMoves_->push_back(s);
                }
            }
        }
        if(std::find(moves_->begin(), moves_->end(), checker->getPosition()) != moves_->end()) {
            validMoves_->push_back(checker->getPosition());
        }

    }

    void AbstractPiece::reset() {
        pinnedBy_ = nullptr;
        protectedBy_ = nullptr;
        moves_->clear();
        validMoves_->clear();
    }

    string AbstractPiece::toString() const {
        return std::string(1 , color_ == chess::WHITE ? toupper(kind_) : tolower(kind_));
    }

    bool AbstractPiece::operator==(const chess::AbstractPiece &that) const {
        return getPosition() == that.getPosition() && getKind() == that.getKind() && getColor() == that.getColor();
    }

    int AbstractPiece::getColor() const {
        return color_;
    }

    Square AbstractPiece::getPosition() const {
        return position_;
    }

    void AbstractPiece::setHasMoved(bool h) {
        moved_ = h;
    }

    bool AbstractPiece::equal_to(const AbstractPiece & that) const {
        return getColor() == that.getColor() && getPosition() == that.getPosition() && getKind() == that.getKind();
    }

    void Pawn::updateMoves() {
        //cut moves
        int ycut = color_ == WHITE ? position_.y + 1 : position_.y - 1;
        Square cutpos(position_.x-1, ycut);
        if(board_->getPieceOnCoords(cutpos) != nullptr) {
            moves_->push_back(cutpos);
        }

        if(board_->getEnPassantSquare() == cutpos) {
            moves_->push_back(cutpos);
        }

        Square cutpos2(position_.x+1, ycut);
        if(board_->getPieceOnCoords(cutpos2) != nullptr) {
            moves_->push_back(cutpos2);
        }

        if(board_->getEnPassantSquare() == cutpos2) {
            moves_->push_back(cutpos2);
        }


        Square moveForward(position_.x, ycut);
        moves_->push_back(moveForward);

        if(!moved_) {
            Square moveByTwo(position_.x, color_ == WHITE ? position_.y + 2 : position_.y - 2);
            moves_->push_back(moveByTwo);
        }
    }

    void Pawn::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        //cut moves
        int ycut = color_ == WHITE ? position_.y + 1 : position_.y - 1;
        for(int i = -1; i < 2; i += 2) {
            Square cutpos(position_.x + i, ycut);
            if(board_->coordOutOfBounds(cutpos)) continue;
            shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(cutpos);
            if(p != nullptr && p->getColor() != getColor()) {
                if(!onlyPinsAndProtects) validMoves_->push_back(cutpos);
            } else if (p != nullptr && p->getColor() == getColor()) {
                p->setProtectedBy(this);
            } else if(board_->getEnPassantSquare() == cutpos) {
                if(!onlyPinsAndProtects) validMoves_->push_back(cutpos);
            }
        }

        Square moveForward(position_.x, ycut);
        if(board_->getPieceOnCoords(moveForward) == nullptr) {
            if(!onlyPinsAndProtects) validMoves_->push_back(moveForward);
        }

        Square moveByTwo(position_.x, color_ == WHITE ? position_.y + 2 : position_.y - 2);
        if(!moved_ && board_->getPieceOnCoords(moveByTwo) == nullptr && board_->getPieceOnCoords(moveForward) == nullptr) {
            if(!onlyPinsAndProtects) validMoves_->push_back(moveByTwo);
        }
    }

    vector<Square> Pawn::getSquaresAttacked() const {
        vector<Square> v;
        int ycut = color_ == WHITE ? position_.y + 1 : position_.y - 1;
        Square pos1(position_.x-1, ycut);
        v.push_back(pos1);

        Square pos2(position_.x+1, ycut);
        v.push_back(pos2);
        return v;
    }

    int Pawn::getId() const {
        return id_;
    }

    void Rook::updateMoves() {
        for(int* i: rookMoves) {
            int dx = position_.x;
            int dy = position_.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board_->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) continue;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
                if(p != nullptr && p->getColor() == getColor()) {
                    breakInner = true;
                    continue;
                }
                moves_->push_back(newPos);
            }
        }
    }

    void Rook::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: rookMoves) {
            int dx = position_.x;
            int dy = position_.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board_->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx,dy);
                shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
                if(p != nullptr) {
                    breakInner = true;
                    if(p->getColor() == getColor()) {
                        p->setProtectedBy(this);
                        continue;
                    } else {
                        if(board_->checkPinnedPiece(this, p.get())) {
                            p->setPinnedBy(this);
                            p->updateValidMovesWhilePinned();
                        }
                    }
                }
                if(!onlyPinsAndProtects) validMoves_->push_back(newPos);
            }
        }
    }

    vector<Square> Rook::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: rookMoves) {
            int dx = position_.x;
            int dy = position_.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board_->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
                if((p != nullptr && p->getKind() != KING) || (p != nullptr && p->getKind() == KING && p->getColor() == getColor())) {
                    breakInner = true;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void Knight::updateMoves() {
        for(int* i: knightMoves) {
            int dx = position_.x;
            int dy = position_.y;
            Square newPos(dx+i[0], dy+i[1]);
            shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
            if(!board_->coordOutOfBounds(newPos) && (p == nullptr || (p != nullptr && p->getColor() != getColor()))) {
                moves_->push_back(newPos);
            }
        }
    }

    void Knight::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: knightMoves) {
            int dx = position_.x;
            int dy = position_.y;
            Square newPos(dx+i[0], dy+i[1]);
            shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
            if(!board_->coordOutOfBounds(newPos)) {
                if(p != nullptr && p->getColor() == getColor()) {
                    p->setProtectedBy(this);
                    continue;
                }
                if(!onlyPinsAndProtects) validMoves_->push_back(newPos);
            }
        }
    }

    vector<Square> Knight::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: knightMoves) {
            int dx = position_.x;
            int dy = position_.y;
            Square newPos(dx+i[0], dy+i[1]);
            shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
            if(!board_->coordOutOfBounds(newPos)) {
                if(p != nullptr && p->getColor() == getColor()) {
                    continue;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void Bishop::updateMoves() {
        for(int* i: bishopMoves) {
            int dx = position_.x;
            int dy = position_.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board_->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) continue;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
                if(p != nullptr && p->getColor() == getColor()) {
                    breakInner = true;
                    continue;
                }
                moves_->push_back(newPos);
            }
        }
    }

    void Bishop::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: bishopMoves) {
            int dx = position_.x;
            int dy = position_.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board_->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
                if(p != nullptr) {
                    breakInner = true;
                    if(p->getColor() == getColor()) {
                        p->setProtectedBy(this);
                        continue;
                    } else {
                        if(board_->checkPinnedPiece(this, p.get())) {
                            p->setPinnedBy(this);
                            p->updateValidMovesWhilePinned();
                        }
                    }
                }
                if(!onlyPinsAndProtects) validMoves_->push_back(newPos);
            }
        }
    }

    vector<Square> Bishop::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: bishopMoves) {
            int dx = position_.x;
            int dy = position_.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board_->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
                if((p != nullptr && p->getKind() != KING) || (p != nullptr && p->getKind() == KING && p->getColor() == getColor())) {
                    breakInner = true;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void Queen::updateMoves() {
        for(int* i: queenKingMoves) {
            int dx = position_.x;
            int dy = position_.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board_->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) continue;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
                if(p != nullptr && p->getColor() == getColor()) {
                    breakInner = true;
                    continue;
                }
                moves_->push_back(newPos);
            }
        }
    }

    void Queen::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: queenKingMoves) {
            int dx = position_.x;
            int dy = position_.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board_->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
                if(p != nullptr) {
                    breakInner = true;
                    if(p->getColor() == getColor()) {
                        p->setProtectedBy(this);
                        continue;
                    } else {
                        if(board_->checkPinnedPiece(this, p.get())) {
                            p->setPinnedBy(this);
                            p->updateValidMovesWhilePinned();
                        }
                    }
                }
                if(!onlyPinsAndProtects) validMoves_->push_back(newPos);
            }
        }
    }

    vector<Square> Queen::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: queenKingMoves) {
            int dx = position_.x;
            int dy = position_.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board_->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
                if((p != nullptr && p->getKind() != KING) || (p != nullptr && p->getKind() == KING && p->getColor() == getColor())) {
                    breakInner = true;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void King::updateMoves() {
        for(int* i: queenKingMoves) {
            Square newPos(position_.x+i[0], position_.y+i[1]);
            if(board_->coordOutOfBounds(newPos)) {
                continue;
            }
            shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
            if(p != nullptr && p->getColor() == getColor()) continue;
            moves_->push_back(newPos);
        }

        if(!hasMoved() && board_->isPlayerInCheck() != getColor() && board_->canCastle()) {
            vector<shared_ptr<AbstractPiece>> rooks = board_->getPiecesOfColorAndKind(color_, ROOK);
            for(const shared_ptr<AbstractPiece>& r: rooks) {
                if(!r->hasMoved()) {
                    vector<Square> squaresBetween = board_->getSquaresBetween(r.get(), this);
                    if(squaresBetween.size() == 2) {
                        //short castle

                        bool breakinner = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(board_->getPieceOnCoords(i) != nullptr) {
                                breakinner = true;
                                break;
                            }
                        }
                        if(breakinner) continue;

                        int dx = r->getPosition().x > position_.x ? r->getPosition().x - 1: r->getPosition().x + 1;
                        Square newPos(dx, position_.y);
                        moves_->push_back(newPos);
                    } else {
                        //long castle

                        bool breakinner = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(board_->getPieceOnCoords(i) != nullptr) {
                                breakinner = true;
                                break;
                            }
                        }
                        if(breakinner) continue;


                        int dx = r->getPosition().x > position_.x ? r->getPosition().x - 2: r->getPosition().x + 2;
                        Square newPos(dx, position_.y);
                        moves_->push_back(newPos);
                    }
                }
            }
        }
    }

    void King::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: queenKingMoves) {
            Square newPos(position_.x+i[0], position_.y+i[1]);
            if(board_->coordOutOfBounds(newPos) || board_->isSquareUnderAttack(invertColor(getColor()), newPos)) {
                continue;
            }
            shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
            if(p != nullptr && p->getColor() == getColor()) {
                p->setProtectedBy(this);
                continue;
            }
            if(p != nullptr && p->getProtectedBy() != nullptr) continue;
            validMoves_->push_back(newPos);
        }

        if(!hasMoved() && board_->isPlayerInCheck() != getColor() && board_->canCastle()) {
            vector<shared_ptr<AbstractPiece>> rooks = board_->getPiecesOfColorAndKind(color_, ROOK);
            for(const shared_ptr<AbstractPiece>& r: rooks) {
                if(!r->hasMoved()) {
                    vector<Square> squares = board_->getSquaresBetween(r.get(), this);
                    bool breakinner = false;
                    for(Square i: squares) {
                        if(board_->isSquareUnderAttack(invertColor(color_), i)) {
                            breakinner = true;
                        }
                    }
                    if(breakinner) continue;
                    vector<Square> squaresBetween = board_->getSquaresBetween(r.get(), this);
                    if(squaresBetween.size() == 2) {
                        //short castle

                        bool breakin = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(board_->getPieceOnCoords(i) != nullptr) {
                                breakin = true;
                                break;
                            }
                        }
                        if(breakin) continue;

                        int dx = r->getPosition().x > position_.x ? r->getPosition().x - 1: r->getPosition().x + 1;
                        Square newPos(dx, position_.y);
                        validMoves_->push_back(newPos);
                    } else {
                        //long castle

                        bool breakin = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(board_->getPieceOnCoords(i) != nullptr) {
                                breakin = true;
                                break;
                            }
                        }
                        if(breakin) continue;

                        int dx = r->getPosition().x > position_.x ? r->getPosition().x - 2: r->getPosition().x + 2;
                        Square newPos(dx, position_.y);
                        validMoves_->push_back(newPos);
                    }
                }
            }
        }
    }

    vector<Square> King::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: queenKingMoves) {
            Square newPos(position_.x+i[0], position_.y+i[1]);
            if(board_->coordOutOfBounds(newPos)) {
                continue;
            }
            shared_ptr<AbstractPiece> p = board_->getPieceOnCoords(newPos);
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

        KriegspielAction::KriegspielAction
        (ActionId id, pair<shared_ptr<AbstractPiece>, Square> move, Square originalPos):
        Action(id), moveFrom_(originalPos), move_(std::move(move)) {
        }

        KriegspielAction::KriegspielAction(ActionId id):
        Action(id), moveFrom_(-1, -1),
        move_(pair<shared_ptr<AbstractPiece>, Square>(nullptr, Square(-1, -1))) {

        }

        Square KriegspielAction::movingFrom() const {
            return moveFrom_;
        }

        KriegspielDomain::KriegspielDomain(unsigned int maxDepth, unsigned int legalMaxDepth, chess::BOARD b):Domain(maxDepth, 2) {
            vector<double> rewards(2);
            vector<shared_ptr<Observation>> Obs{make_shared<Observation>(NO_OBSERVATION), make_shared<Observation>(NO_OBSERVATION)};
            Outcome o(make_shared<KriegspielState>(this, legalMaxDepth, b), Obs, rewards);
            rootStatesDistribution_.push_back(pair<Outcome, double>(move(o), 1.0));
            maxUtility_ = 1;
        }
        KriegspielObservation::KriegspielObservation(int id):Observation(id) {}
        KriegspielState::KriegspielState(GTLib2::Domain *domain, int legalMaxDepth, chess::BOARD b):State(domain), enPassantSquare_(-1, -1) {
            moveHistory_ = make_shared<vector<shared_ptr<KriegspielAction>>>();
            attemptedMoveHistory_ = make_shared<vector<shared_ptr<KriegspielAction>>>();
            playerOnTheMove_ = chess::WHITE;
            legalMaxDepth = legalMaxDepth;
            initBoard(b);
            updateState(chess::WHITE);
        }

        KriegspielState::KriegspielState(Domain *domain, int legalMaxDepth, string s): State(domain), enPassantSquare_(-1, -1) {
            moveHistory_ = make_shared<vector<shared_ptr<KriegspielAction>>>();
            attemptedMoveHistory_ = make_shared<vector<shared_ptr<KriegspielAction>>>();
            playerOnTheMove_ = chess::WHITE;
            legalMaxDepth = legalMaxDepth;
            initBoard(std::move(s));
            updateAllPieces();
        }

        KriegspielState::KriegspielState(GTLib2::Domain *domain, int legalMaxDepth, int x, int y, shared_ptr<vector<shared_ptr<chess::AbstractPiece>>> pieces,
                                         Square enPassantSquare, shared_ptr<vector<shared_ptr<KriegspielAction>>> moves, int p, bool castle,
                                         shared_ptr<vector<shared_ptr<KriegspielAction>>> attemptedMoves):State(domain), enPassantSquare_(enPassantSquare) {
            xSize_ = x;
            ySize_ = y;
            pieces = std::move(pieces);
            moveHistory_ = std::move(moves);
            legalMaxDepth = legalMaxDepth;
            attemptedMoveHistory_ = std::move(attemptedMoves);
            canPlayerCastle_ = castle;

            //rebind to new board state
            for(shared_ptr<AbstractPiece>& piece: *pieces) {
                piece->setBoard(this);
            }
            playerOnTheMove_ = p;
        }

        pair<shared_ptr<AbstractPiece>, Square> KriegspielAction::getMove() const {
            return move_;
        }

        bool KriegspielAction::operator==(const GTLib2::Action &that) const {
            const auto rhsAction = dynamic_cast<const KriegspielAction *>(&that);
            return move_.first->equal_to(*rhsAction->move_.first) && move_.second == rhsAction->move_.second && movingFrom() == rhsAction->movingFrom();
        }

        size_t KriegspielAction::getHash() const {
            std::hash<string> h;
            string s = move_.first->toString() + chess::coordToString(move_.second) + chess::coordToString(movingFrom());
            return h(s);
        }

        shared_ptr<KriegspielAction> KriegspielAction::clone() const {
            Square s = Square(getMove().second.x, getMove().second.y);
            shared_ptr<AbstractPiece> p = getMove().first->clone();
            Square f = Square(movingFrom().x, movingFrom().y);
            auto a = make_shared<KriegspielAction>(id_, pair<shared_ptr<AbstractPiece>, chess::Square>(p, s), f);
            return a;
        }

        string KriegspielDomain::getInfo() const {
            return "************ Kriegspiel *************\n" +
                   rootStatesDistribution_[0].first.state_->toString() + "\n";
        }

        KriegspielDomain::KriegspielDomain(unsigned int maxDepth, unsigned int legalMaxDepth, string s): Domain(maxDepth, 2) {
            vector<double> rewards(2);
            vector<shared_ptr<Observation>> Obs{make_shared<Observation>(NO_OBSERVATION), make_shared<Observation>(-1)};
            Outcome o(make_shared<KriegspielState>(this, legalMaxDepth, s), Obs, rewards);
            rootStatesDistribution_.push_back(pair<Outcome, double>(move(o), 1.0));
        }

        vector<shared_ptr<Action>> KriegspielState::getAvailableActionsFor(Player player) const {
            auto list = vector<shared_ptr<Action>>();
            if(player != playerOnTheMove_ || gameHasEnded_ || moveHistory_->size() == legalMaxDepth_) {
                return list;
            }
            int count = 0;
            for (shared_ptr<AbstractPiece>p: *pieces_) {
                if(p->getColor() != player) continue;
                auto moves = p->getAllMoves();
                for(Square move: *moves) {
                    auto ksa = make_shared<KriegspielAction>(count, std::pair<shared_ptr<AbstractPiece>, Square>(p, move), p->getPosition());
                    if(std::find(attemptedMoveHistory_->begin(), attemptedMoveHistory_->end(), ksa) == attemptedMoveHistory_->end()) {
                        list.push_back(ksa);
                        ++count;
                    }
                }
            }
            return list;
        }

        void KriegspielState::promote(shared_ptr<AbstractPiece> p, Square pos) const {
            auto q = make_shared<chess::Queen>(chess::QUEEN, p->getColor(), pos, this);
            for(auto It = pieces_->begin(); It != pieces_->end();) {
                if(*(*It) == *p) {
                    pieces_->erase(It);
                    pieces_->push_back(q);
                    break;
                }
                else {
                    ++It;
                }
            }
        }

        vector<shared_ptr<AbstractPiece>> KriegspielState::getPiecesOfColor(int color) const {
            vector<shared_ptr<AbstractPiece>> v;
            for(shared_ptr<AbstractPiece>& p: *pieces_) {
                if(p->getColor() == color) {
                    v.push_back(p);
                }
            }
            return v;
        }

        int KriegspielState::isPlayerInCheck() const {
            return playerInCheck_;
        }

        vector<shared_ptr<AbstractPiece>> KriegspielState::getCheckingFigures() const {
            return checkingFigures_;
        }

        int KriegspielState::checkGameOverCheck(int c) const {
            int inCheck = -1;
            shared_ptr<AbstractPiece> king = getPiecesOfColorAndKind(c, chess::KING)[0];
            for(const shared_ptr<AbstractPiece>& p: *pieces_) {
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
            int kingColor = playerOnTheMove_;
            playerInCheck_ = -1;
            shared_ptr<AbstractPiece> king = getPiecesOfColorAndKind(kingColor, chess::KING)[0];
            for(const shared_ptr<AbstractPiece>& p: *pieces_) {
                if(p->getColor() == kingColor) continue;
                Square kingPos = king->getPosition();
                vector<Square> attackSquares = p->getSquaresAttacked();
                if(std::find(attackSquares.begin(), attackSquares.end(), kingPos) != attackSquares.end()) {
                    playerInCheck_ = kingColor;
                    checkingFigures_.push_back(p);
                }
            }
        }

        bool KriegspielState::canCastle() const {
            return canPlayerCastle_;
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
            for(; ((rook = getPieceOnCoords(Square(dx, dy))) == nullptr); dx += incrementX);
            int newRookX = pos.x > rook->getPosition().x ? pos.x + 1 : pos.x - 1;
            Square newRookPos(newRookX, rook->getPosition().y);
            rook->move(newRookPos);
        }

        bool KriegspielState::move(KriegspielAction * a) {
            pair<shared_ptr<AbstractPiece>, Square> move = a->getMove();
            shared_ptr<AbstractPiece> p = getPieceOnCoords(move.first->getPosition());
            Square pos = move.second;

            if(std::find(p->getAllValidMoves()->begin(), p->getAllValidMoves()->end(), pos) != p->getAllValidMoves()->end()) {
                //move valid
                shared_ptr<AbstractPiece> checkCut = getPieceOnCoords(pos);
                if(checkCut != nullptr) {
                    lastCut_ = 30 + checkCut->getPosition().x + ((checkCut->getPosition().y-1) * ySize_);
                    for(auto It = pieces_->begin(); It != pieces_->end();) {
                        if(*(*It) == *checkCut) {
                            pieces_->erase(It);
                            break;
                        }
                        else {
                            ++It;
                        }
                    }
                }

                if(pos.x == enPassantSquare_.x && pos.y == enPassantSquare_.y) {
                    int pawnColor = chess::invertColor(p->getColor());
                    int y = pawnColor == chess::WHITE ? pos.y + 1 : pos.y - 1;
                    Square enPassPos(pos.x, y);
                    shared_ptr<AbstractPiece> enPassPawn = getPieceOnCoords(enPassPos);
                    if(enPassPawn != nullptr) {
                        for(auto It = pieces_->begin(); It != pieces_->end();) {
                            if(*(*It) == *enPassPawn) {
                                pieces_->erase(It);
                                break;
                            }
                            else {
                                ++It;
                            }
                        }
                    }
                }

                if(p->getKind() == chess::KING && abs(a->movingFrom().x - pos.x) > 1) {
                    castle(a);
                }
                p->move(pos);
                checkPlayerInCheck();
                if(p->getKind() == chess::PAWN && ((pos.y == getYSize() && p->getColor() == chess::WHITE) || (pos.y == 1 && p->getColor() == chess::BLACK))) {
                    promote(p, pos);
                }
                return true;
            } else {
                return false;
            }
        }

        vector<double> KriegspielState::checkGameOver() const {
            vector<double> rewards(2);
            vector<shared_ptr<AbstractPiece>> whitePieces = getPiecesOfColor(0);
            vector<shared_ptr<AbstractPiece>> blackPieces = getPiecesOfColor(1);
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
            int isCheck = checkGameOverCheck(playerOnTheMove_);

            for(const shared_ptr<AbstractPiece>& p: *pieces_) {
                hasMoves[p->getColor()] = hasMoves[p->getColor()] ? true : !p->getAllValidMoves()->empty();
            }

            if(isCheck == playerOnTheMove_) {
                if(!hasMoves[playerOnTheMove_]) {
                    rewards[playerOnTheMove_] = 0;
                    rewards[chess::invertColor(playerOnTheMove_)] = 1;
                } else if(moveHistory_->size() == legalMaxDepth_) {
                    rewards[chess::WHITE] = 0.5;
                    rewards[chess::BLACK] = 0.5;
                } else if(moveHistory_->size() + attemptedMoveHistory_->size() == domain_->getMaxDepth()) {
                    rewards[chess::WHITE] = 0.5;
                    rewards[chess::BLACK] = 0.5;
                }
            } else if(!hasMoves[playerOnTheMove_]) {
                rewards[chess::WHITE] = 0.5;
                rewards[chess::BLACK] = 0.5;
            } else if(moveHistory_->size() == legalMaxDepth_) {
                rewards[chess::WHITE] = 0.5;
                rewards[chess::BLACK] = 0.5;
            }

            return rewards;
        }

        Square KriegspielState::getEnPassantSquare() const {
            return enPassantSquare_;
        }



        OutcomeDistribution KriegspielState::performActions(
                const vector<pair<Player, shared_ptr<GTLib2::Action>>> &actions) const {
            auto a1 = dynamic_cast<KriegspielAction*>(actions[0].second.get());
            auto a2 = dynamic_cast<KriegspielAction*>(actions[1].second.get());
            vector<shared_ptr<Observation>> observations(2);
            vector<double> rewards(2);
            shared_ptr<KriegspielState> s;
            int nextMove = playerOnTheMove_;
            Square enPassSquare(-1, -1);
            KriegspielAction* a = a1 ? a1 : a2;
            shared_ptr<vector<shared_ptr<AbstractPiece>>> pieces = copyPieces();
            shared_ptr<vector<shared_ptr<KriegspielAction>>> history = copyMoveHistory();
            shared_ptr<vector<shared_ptr<KriegspielAction>>> attemptedmoves = copyAttemptedMoves();
            s = make_shared<KriegspielState>(domain_, legalMaxDepth_, xSize_, ySize_, pieces, enPassantSquare_, history, nextMove, canPlayerCastle_, attemptedmoves);
            s->updateState(playerOnTheMove_);
            shared_ptr<KriegspielAction> ac = a->clone();
            if(s->move(a)) {
                s->addToHistory(ac);
                enPassSquare = checkEnPassant(a);
                nextMove = chess::invertColor(playerOnTheMove_);
                s->setPlayerOnMove(nextMove);
                s->setEnPassant(enPassSquare);
                s->updateState(nextMove);
                rewards = s->checkGameOver();
                observations[playerOnTheMove_] = make_shared<Observation>(1);
                observations[chess::invertColor(playerOnTheMove_)] = make_shared<Observation>(s->calculateObservation(chess::invertColor(playerOnTheMove_)));
                int x = 4;
            } else {
                s->addToAttemptedMoves(ac);
                nextMove = playerOnTheMove_;
                observations[playerOnTheMove_] = make_shared<Observation>(a->getId());
                observations[chess::invertColor(playerOnTheMove_)] = make_shared<Observation>(NO_OBSERVATION);
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
            if(xSize_ != rhsState->xSize_ || ySize_ != rhsState->ySize_) {
                return false;
            }
            for(int i = 1; i <= xSize_; i++) {
                for(int j = 1; j <= ySize_; j++) {
                    shared_ptr<AbstractPiece> p1 = getPieceOnCoords(Square(i, j));
                    shared_ptr<AbstractPiece> p2 = getPieceOnCoords(Square(i, j));
                    if(p1 == nullptr && p2 == nullptr) continue;
                    if((p1 != nullptr && p2 == nullptr) || (p1 == nullptr && p2 != nullptr)) return false;
                    if(!(*p1 == *p2)) return false;
                }
            }
            return true;
        }

        size_t KriegspielState::getHash() const {
            std::hash<string> h;
            return h(toString());
        }

        string KriegspielState::toString() const {
            //DEPRECATED
            vector<vector<string>> board;
            for(int j = 0; j < ySize_; j++) {
                vector<string> row;
                for(int i = 0; i < xSize_; i++) {
                    row.emplace_back("_");
                }
                board.push_back(row);
            }

            for(shared_ptr<AbstractPiece>p: *pieces_) {
                Square pos = p->getPosition();
                pos.y -= 1;
                pos.x -= 1;
                board.at(pos.y).at(pos.x) = p->toString();
            }

            string s;
            for(int i = ySize_-1; i >= 0; i--) {
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
            for(const shared_ptr<AbstractPiece>& p: *pieces_) {
                if(p->getPosition() == pos) {
                    return p;
                }
            }
            return nullptr;
        }

        bool KriegspielState::coordOutOfBounds(Square pos) const {
            return pos.x > xSize_ || pos.y > ySize_ || pos.x < 1 || pos.y < 1;
        }

        void KriegspielState::updateAllPieces() const {
            for(shared_ptr<AbstractPiece>& p: *pieces_) {
                p->update();
            }
        }

        void KriegspielState::resetAllPieces() {
            for(shared_ptr<AbstractPiece>& p: *pieces_) {
                p->reset();
            }
        }

        void KriegspielState::updatePiecesOfColor(int color) const {
            vector<shared_ptr<AbstractPiece>> v = getPiecesOfColor(color);
            for(shared_ptr<AbstractPiece>& p: v) {
                p->update();
            }
        }

        vector<shared_ptr<AbstractPiece>> KriegspielState::getPiecesOfColorAndKind(int c, chess::pieceName n) const {
            vector<shared_ptr<AbstractPiece>> v;
            for(const shared_ptr<AbstractPiece>& p: *pieces_) {
                if(p->getColor() == c && p->getKind() == n) {
                    v.push_back(p);
                }
            }
            return v;
        }

        bool KriegspielState::isSquareUnderAttack(int color, Square square) const {
            vector<shared_ptr<AbstractPiece>> pieces = getPiecesOfColor(color);
            for(const shared_ptr<AbstractPiece>& p: pieces) {
                vector<Square> attackedSquares = p->getSquaresAttacked();
                if(std::find(attackedSquares.begin(), attackedSquares.end(), square) != attackedSquares.end()) {
                    return true;
                }
            }
            return false;
        }

        void KriegspielState::clearBoard() {
            pieces_->clear();
        }

        void KriegspielState::insertPiece(shared_ptr<AbstractPiece> p) {
            pieces_->push_back(p);
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

            for(; !coordOutOfBounds(Square(dx, dy)); dx += incrementX, dy += incrementY) {
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
                if(coordOutOfBounds(newPos)) {
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
            shared_ptr<AbstractPiece> king = getPiecesOfColorAndKind(kingColor, chess::pieceName::KING)[0];
            vector<Square> squaresToCheck = getSquaresPierced(pinned, pinner);
            for(Square i: squaresToCheck) {
                shared_ptr<AbstractPiece> p = getPieceOnCoords(i);
                if(p != nullptr) {
                    return p == king;
                }
            }
            return false;
        }

        void KriegspielState::initBoard(chess::BOARD b) {
            chess::BoardFactory bf;
            chess::boardInfo binfo = bf.create(b, this);
            pieces_ = binfo.pieces;
            xSize_ = binfo.x;
            ySize_ = binfo.y;
            canPlayerCastle_ = binfo.castle;
        }

        int KriegspielState::getXSize() const {
            return xSize_;
        }

        int KriegspielState::getYSize() const {
            return ySize_;
        }

        void KriegspielState::setPlayerOnMove(int p) {
            playerOnTheMove_ = p;
        }

        void KriegspielState::setGameHasEnded(bool gameHasEnded) {
            gameHasEnded = gameHasEnded;
        }

        shared_ptr<vector<shared_ptr<AbstractPiece>>> KriegspielState::copyPieces() const {
            shared_ptr<vector<shared_ptr<AbstractPiece>>> v = make_shared<vector<shared_ptr<AbstractPiece>>>();
            for(const shared_ptr<AbstractPiece>& p: *pieces_) {
                auto asdf = p->clone();
                v->push_back(asdf);
            }
            return v;
        }

        shared_ptr<vector<shared_ptr<KriegspielAction>>> KriegspielState::copyMoveHistory() const {
            shared_ptr<vector<shared_ptr<KriegspielAction>>> v = make_shared<vector<shared_ptr<KriegspielAction>>>();
            for(const shared_ptr<KriegspielAction>& a: *moveHistory_) {
                auto asdf = a->clone();
                v->push_back(asdf);
            }
            return v;
        }

        void KriegspielState::addToHistory(shared_ptr<KriegspielAction> a) {
            moveHistory_->push_back(a);
            attemptedMoveHistory_->clear();
        }

        void KriegspielState::setEnPassant(chess::Square s) {
            enPassantSquare_ = s;
            //refresh moves on pawns after setting enpassant square
            vector<shared_ptr<AbstractPiece>> pawns = getPiecesOfColorAndKind(playerOnTheMove_, chess::PAWN);
            for(shared_ptr<AbstractPiece>& p: pawns) {
                p->update();
            }
        }

        void KriegspielState::updateState(int p) {
            playerOnTheMove_ = p;
            resetAllPieces();
            updatePiecesOfColor(chess::invertColor(playerOnTheMove_));
            checkPlayerInCheck();
            updatePiecesOfColor(playerOnTheMove_);

            //update kings again to remove moves that would cut protected pieces
            shared_ptr<AbstractPiece> whiteKing = getPiecesOfColorAndKind(chess::WHITE, chess::KING)[0];
            shared_ptr<AbstractPiece> blackKing = getPiecesOfColorAndKind(chess::BLACK, chess::KING)[0];
            whiteKing->reset();
            blackKing->reset();
            whiteKing->update();
            blackKing->update();
        }

        void KriegspielState::initBoard(string s) {
            chess::FenBoardFactory bf;
            chess::boardInfo binfo = bf.create(std::move(s), this);
            pieces_ = binfo.pieces;
            xSize_ = binfo.x;
            ySize_ = binfo.y;
            canPlayerCastle_ = binfo.castle;
            if(binfo.xEnpass) {
                Square sq(binfo.xEnpass, binfo.yEnpass);
                setEnPassant(sq);
            }
        }

        shared_ptr<vector<shared_ptr<KriegspielAction>>> KriegspielState::copyAttemptedMoves() const {
            shared_ptr<vector<shared_ptr<KriegspielAction>>> v = make_shared<vector<shared_ptr<KriegspielAction>>>();
            for(const shared_ptr<KriegspielAction>& a: *attemptedMoveHistory_) {
                auto asdf = a->clone();
                v->push_back(asdf);
            }
            return v;
        }

        void KriegspielState::addToAttemptedMoves(shared_ptr<KriegspielAction> a) {
            attemptedMoveHistory_->push_back(a);
        }

        int KriegspielState::calculateObservation(int player) const {
            auto pawns = getPiecesOfColorAndKind(player, chess::PAWN);
            int toreturn = lastCut_;
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
