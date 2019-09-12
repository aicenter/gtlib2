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

#include <base/random.h>
#include "base/base.h"
#include "domains/kriegspiel.h"

#include "tests/domainsTest.h"
#include "gtest/gtest.h"

namespace GTLib2::domains {

using namespace chess;

TEST(Kriegspiel, stability) {
    domains::KriegspielDomain d(1000, 1000, BOARD::STANDARD);
    auto rootNode = createRootEFGNode(d);
    auto expectedOutcomes = array<double, 10>{0, 0, 1, 0, 0, 1, 0, 1, 0, 0};

    for (int i = 0; i < 10; i++) {
        std::mt19937 gen = std::mt19937(i);
        RandomLeafOutcome r = pickRandomLeaf(rootNode, gen);
        EXPECT_EQ(r.utilities[0] + r.utilities[1], 0);
        EXPECT_EQ(r.utilities[0], expectedOutcomes[i]);
    }
}

TEST(Kriegspiel, pinning) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    EXPECT_EQ(ks->getAvailableActionsFor(0).size(), 20);
    ks->clearBoard();

    //build a model pinning situation
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(5, 1), ks);
    shared_ptr<Bishop> whiteBishop = make_shared<Bishop>(BISHOP, 0, Square(5, 2), ks);
    shared_ptr<Queen> blackQueen = make_shared<Queen>(QUEEN, 1, Square(5, 3), ks);
    ks->insertPiece(whiteKing);
    ks->insertPiece(whiteBishop);
    ks->insertPiece(blackQueen);
    ks->updateAllPieces();

    //the bishops only valid move should be the square whence the bishop is being pinned
    EXPECT_EQ(ks->getPiecesOfColorAndKind(0, BISHOP)[0]->getAllValidMoves()->size(), 1);
}

TEST(Kriegspiel, enPassant) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();

    //build a model enPassant situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(1, 1), ks);
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(1, 8), ks);

    shared_ptr<Pawn> whitePawn = make_shared<Pawn>(PAWN, 0, Square(5, 2), ks, 1);
    shared_ptr<Pawn> blackPawn = make_shared<Pawn>(PAWN, 1, Square(4, 4), ks, 3);
    ks->insertPiece(whitePawn);
    ks->insertPiece(blackPawn);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Pe4") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    auto newBoard = dynamic_cast<domains::KriegspielState *>(newState.state.get());
    //a black pawn's valid move should now be to capture en passant
    Square sq(5, 3);
    auto newBlackPawn = newBoard->getPiecesOfColorAndKind(1, PAWN)[0];
    EXPECT_NE(std::find(newBlackPawn->getAllValidMoves()->begin(),
                        newBlackPawn->getAllValidMoves()->end(),
                        sq), newBlackPawn->getAllValidMoves()->end());

    //make the en passant cut and check figure has been cut
    v.clear();
    for (shared_ptr<Action> a: newBoard->getAvailableActionsFor(1)) {
        string check = a->toString();
        if (a->toString() == "pe3") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome lastState = newState.state->performActions(v)[0].outcome;
    auto lastBoard = dynamic_cast<domains::KriegspielState *>(lastState.state.get());
    EXPECT_EQ(lastBoard->getPiecesOfColor(0).size() + lastBoard->getPiecesOfColor(1).size(), 3);
}

TEST(Kriegspiel, invalidMoving) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();

    //build a model invalid move situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(1, 1), ks);
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(1, 8), ks);


    //put pawns around the white queen
    shared_ptr<Queen> whiteQueen = make_shared<Queen>(QUEEN, 0, Square(8, 1), ks);
    shared_ptr<Pawn> blackPawn = make_shared<Pawn>(PAWN, 1, Square(8, 2), ks, 1);
    shared_ptr<Pawn> blackPawn2 = make_shared<Pawn>(PAWN, 1, Square(7, 2), ks, 3);
    shared_ptr<Pawn> blackPawn3 = make_shared<Pawn>(PAWN, 1, Square(7, 1), ks, 5);
    ks->insertPiece(whiteQueen);
    ks->insertPiece(blackPawn);
    ks->insertPiece(blackPawn2);
    ks->insertPiece(blackPawn3);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    //fetch random invalid move
    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Qb1") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    EXPECT_EQ(newState.privateObservations[0]->getId(), v[0].get()->getId());
    EXPECT_EQ(newState.privateObservations[1]->getId(), NO_OBSERVATION);
}

TEST(Kriegspiel, checking) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();

    //build a model checking situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(1, 1), ks);
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(1, 8), ks);

    shared_ptr<Queen> whiteQueen = make_shared<Queen>(QUEEN, 0, Square(5, 1), ks);
    ks->insertPiece(whiteQueen);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    //fetch checking move
    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Qe8") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    auto newBoard = dynamic_cast<domains::KriegspielState *>(newState.state.get());
    EXPECT_TRUE(newBoard->isPlayerInCheck());
    auto newBlackKing = newBoard->getPiecesOfColorAndKind(1, KING)[0];
    EXPECT_EQ(newBlackKing->getAllValidMoves()->size(), 2);
}

TEST(Kriegspiel, doublechecking) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();

    //build a model doublechecking situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(1, 1), ks);
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(5, 8), ks);

    shared_ptr<Queen> whiteQueen = make_shared<Queen>(QUEEN, 0, Square(5, 1), ks);
    shared_ptr<Bishop> whiteBishop = make_shared<Bishop>(BISHOP, 0, Square(5, 2), ks);
    shared_ptr<Bishop> blackBishop = make_shared<Bishop>(BISHOP, 1, Square(4, 8), ks);
    shared_ptr<Rook> blackRook = make_shared<Rook>(ROOK, 1, Square(6, 8), ks);
    ks->insertPiece(whiteQueen);
    ks->insertPiece(whiteBishop);
    ks->insertPiece(blackBishop);
    ks->insertPiece(blackRook);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    //fetch double check move
    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Bb5") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    auto newBoard = dynamic_cast<domains::KriegspielState *>(newState.state.get());
    EXPECT_TRUE(newBoard->isPlayerInCheck());
    //the only valid move for black should be Kf7
    auto newBlackBishop = newBoard->getPiecesOfColorAndKind(1, BISHOP)[0];
    auto newBlackRook = newBoard->getPiecesOfColorAndKind(1, ROOK)[0];
    auto newBlackKing = newBoard->getPiecesOfColorAndKind(1, KING)[0];
    EXPECT_TRUE(newBlackBishop->getAllValidMoves()->empty());
    EXPECT_TRUE(newBlackRook->getAllValidMoves()->empty());
    EXPECT_EQ(newBlackKing->getAllValidMoves()->size(), 1);
}

TEST(Kriegspiel, castling) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();

    //build a model castling situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(5, 1), ks);
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(2, 8), ks);

    shared_ptr<Rook> whiteRookLong = make_shared<Rook>(ROOK, 0, Square(1, 1), ks);
    shared_ptr<Rook> whiteRookShort = make_shared<Rook>(ROOK, 0, Square(8, 1), ks);

    ks->insertPiece(whiteRookLong);
    ks->insertPiece(whiteRookShort);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    //the white king should have 5 moves + 2 castling moves
    EXPECT_EQ(whiteKing->getAllValidMoves()->size(), 7);

    //fetch castling move
    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Kg1") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    auto newBoard = dynamic_cast<GTLib2::domains::KriegspielState *>(newState.state.get());
    AbstractPiece *newWhiteKing = newBoard->getPiecesOfColorAndKind(0, KING)[0].get();
    AbstractPiece *newWhiteRookShort = newBoard->getPiecesOfColorAndKind(0, ROOK)[1].get();
    EXPECT_EQ(newWhiteKing->getPosition().x, 7);
    EXPECT_EQ(newWhiteKing->getPosition().y, 1);
    EXPECT_EQ(newWhiteRookShort->getPosition().x, 6);
    EXPECT_EQ(newWhiteRookShort->getPosition().y, 1);

    domains::KriegspielDomain d2(4, 4, BOARD::STANDARD);
    shared_ptr<State> s2 = d2.getRootStatesDistribution()[0].outcome.state;
    auto ks2 = dynamic_cast<domains::KriegspielState *>(s2.get());
    ks2->clearBoard();
    whiteKing = make_shared<King>(KING, 0, Square(5, 1), ks);
    blackKing = make_shared<King>(KING, 1, Square(2, 8), ks);

    whiteRookLong = make_shared<Rook>(ROOK, 0, Square(1, 1), ks);
    whiteRookShort = make_shared<Rook>(ROOK, 0, Square(8, 1), ks);

    ks2->insertPiece(whiteRookLong);
    ks2->insertPiece(whiteRookShort);
    ks2->insertPiece(whiteKing);
    ks2->insertPiece(blackKing);
    ks2->updateAllPieces();

    //fetch castling move
    v.clear();
    for (shared_ptr<Action> a: ks2->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Kc1") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome lastState = ks2->performActions(v)[0].outcome;
    auto lastBoard = dynamic_cast<GTLib2::domains::KriegspielState *>(lastState.state.get());
    AbstractPiece *newWhiteKing2 = lastBoard->getPiecesOfColorAndKind(0, KING)[0].get();
    AbstractPiece *newWhiteRookLong = lastBoard->getPiecesOfColorAndKind(0, ROOK)[0].get();
    EXPECT_EQ(newWhiteKing2->getPosition().x, 3);
    EXPECT_EQ(newWhiteKing2->getPosition().y, 1);
    EXPECT_EQ(newWhiteRookLong->getPosition().x, 4);
    EXPECT_EQ(newWhiteRookLong->getPosition().y, 1);
}

TEST(Kriegspiel, castleDeny) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();

    //build a model castling situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(5, 1), ks);
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(2, 8), ks);

    shared_ptr<Rook> whiteRookLong = make_shared<Rook>(ROOK, 0, Square(1, 1), ks);
    shared_ptr<Rook> blackRook = make_shared<Rook>(ROOK, 1, Square(7, 8), ks);
    shared_ptr<Rook> whiteRookShort = make_shared<Rook>(ROOK, 0, Square(8, 1), ks);

    ks->insertPiece(whiteRookLong);
    ks->insertPiece(whiteRookShort);
    ks->insertPiece(blackRook);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    //the white king should have 5 moves + 1 castling move
    EXPECT_EQ(whiteKing->getAllValidMoves()->size(), 6);

    //fetch castling move
    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Kg1") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    //short castle should be invalid
    Outcome newState = ks->performActions(v)[0].outcome;
    EXPECT_EQ(newState.privateObservations[0]->getId(), v[0].get()->getId());
    EXPECT_EQ(newState.privateObservations[1]->getId(), NO_OBSERVATION);

    //reset the board
    ks->clearBoard();
    whiteKing = make_shared<King>(KING, 0, Square(5, 1), ks);
    blackKing = make_shared<King>(KING, 1, Square(2, 8), ks);
    blackRook = make_shared<Rook>(ROOK, 1, Square(3, 8), ks);
    whiteRookLong = make_shared<Rook>(ROOK, 0, Square(1, 1), ks);
    whiteRookShort = make_shared<Rook>(ROOK, 0, Square(8, 1), ks);

    ks->insertPiece(whiteRookLong);
    ks->insertPiece(whiteRookShort);
    ks->insertPiece(blackRook);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    //fetch castling move
    v.clear();
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Kc1") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    auto newState2 = ks->performActions(v)[0].outcome;
    EXPECT_EQ(newState2.privateObservations[0]->getId(), v[0].get()->getId());
    EXPECT_EQ(newState2.privateObservations[1]->getId(), NO_OBSERVATION);
}

TEST(Kriegspiel, cutting) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();

    //build a model cutting situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(5, 1), ks);
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(2, 8), ks);

    shared_ptr<Rook> blackRook = make_shared<Rook>(ROOK, 1, Square(8, 8), ks);
    shared_ptr<Rook> whiteRook = make_shared<Rook>(ROOK, 0, Square(8, 1), ks);

    ks->insertPiece(whiteRook);
    ks->insertPiece(blackRook);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    //before any move, there should be 4 pieces on the board
    EXPECT_EQ(ks->getPiecesOfColor(0).size() + ks->getPiecesOfColor(1).size(), 4);

    //fetch castling move
    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Rh8") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    //after Rh8 there should be only 3 pieces left - the black rook is cut
    auto newBoard = dynamic_cast<GTLib2::domains::KriegspielState *>(newState.state.get());
    EXPECT_EQ(newBoard->getPiecesOfColor(0).size() + newBoard->getPiecesOfColor(1).size(), 3);
}

TEST(Kriegspiel, gameOverWin) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();

    //build a model game over situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(5, 1), ks);
    //artificially set white kings hasMoved() to true, so the game stops checking for castles
    whiteKing->move(Square(5, 1));
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(2, 8), ks);

    shared_ptr<Rook> whiteRookOne = make_shared<Rook>(ROOK, 0, Square(8, 7), ks);
    shared_ptr<Rook> whiteRookTwo = make_shared<Rook>(ROOK, 0, Square(6, 1), ks);

    ks->insertPiece(whiteRookOne);
    ks->insertPiece(whiteRookTwo);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    //fetch game winning move
    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Rf8") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    EXPECT_EQ(newState.rewards[0], 1);
    EXPECT_EQ(newState.rewards[1], -1);
}

TEST(Kriegspiel, gameOverDraw) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();

    //build a model game drawing situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(4, 6), ks);
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(5, 8), ks);

    shared_ptr<Pawn> whitePawn = make_shared<Pawn>(PAWN, 0, Square(5, 7), ks, 1);

    ks->insertPiece(whitePawn);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    //fetch game winning move
    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Ke6") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    //after Rh8 there should be only 3 pieces left - the black rook is cut
    EXPECT_EQ(newState.rewards[0], 0.0);
    EXPECT_EQ(newState.rewards[1], 0.0);
}

TEST(Kriegspiel, PAWNPromotion) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();

    //build a model game drawing situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(4, 6), ks);
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(1, 1), ks);

    shared_ptr<Pawn> whitePawn = make_shared<Pawn>(PAWN, 0, Square(5, 7), ks, 1);

    ks->insertPiece(whitePawn);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    //fetch game winning move
    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Pe8") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    auto newBoard = dynamic_cast<domains::KriegspielState *>(newState.state.get());
    //after Rh8 there should be only 3 pieces left - the black rook is cut
    EXPECT_EQ(newBoard->getPiecesOfColor(0).size(), 2);
    EXPECT_EQ(newBoard->getPiecesOfColorAndKind(0, QUEEN).size(), 1);
}

TEST(Kriegspiel, protection) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();
    ks->setPlayerOnMove(chess::BLACK);

    shared_ptr<King> whiteKing = make_shared<King>(KING, 0, Square(5, 8), ks);
    shared_ptr<King> blackKing = make_shared<King>(KING, 1, Square(3, 8), ks);

    shared_ptr<Queen> blackQueen = make_shared<Queen>(QUEEN, 1, Square(3, 7), ks);

    ks->insertPiece(blackQueen);
    ks->insertPiece(whiteKing);
    ks->insertPiece(blackKing);
    ks->updateAllPieces();

    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(1)) {
        string check = a->toString();
        if (a->toString() == "qd8") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    auto newBoard = dynamic_cast<domains::KriegspielState *>(newState.state.get());

    v.clear();
    for (shared_ptr<Action> a: newBoard->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Kd8") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    auto newState2 = newBoard->performActions(v)[0].outcome;
    EXPECT_EQ(newState2.privateObservations[0]->getId(), v[0].get()->getId());
    EXPECT_EQ(newState2.privateObservations[1]->getId(), NO_OBSERVATION);
}

TEST(Kriegspiel, randomPin) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();
    ks->setPlayerOnMove(BLACK);

    vector<shared_ptr<AbstractPiece>> pieceV;

    pieceV.emplace_back(make_shared<King>(KING, 0, Square(6, 4), ks));
    pieceV.emplace_back(make_shared<King>(KING, 1, Square(3, 3), ks));

    pieceV.emplace_back(make_shared<Queen>(QUEEN, 0, Square(5, 4), ks));
    pieceV.emplace_back(make_shared<Knight>(KNIGHT, 1, Square(3, 6), ks));
    pieceV.emplace_back(make_shared<Rook>(ROOK, 1, Square(3, 4), ks));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 1, Square(1, 5), ks, 1));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 1, Square(1, 3), ks, 3));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 0, Square(1, 2), ks, 5));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 1, Square(2, 2), ks, 7));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 0, Square(6, 5), ks, 9));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 0, Square(8, 5), ks, 11));

    for (const shared_ptr<AbstractPiece> &p : pieceV) {
        ks->insertPiece(p);
    }
    ks->updateAllPieces();

    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(1)) {
        string check = a->toString();
        if (a->toString() == "pa4") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    auto newBoard = dynamic_cast<domains::KriegspielState *>(newState.state.get());

    v.clear();
    for (shared_ptr<Action> a: newBoard->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Qc6") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome lastState = newBoard->performActions(v)[0].outcome;

    EXPECT_EQ(lastState.privateObservations[0]->getId(), v[0].get()->getId());
    EXPECT_EQ(lastState.privateObservations[1]->getId(), NO_OBSERVATION);
}

TEST(Kriegspiel, randomGameOver) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();
    ks->setPlayerOnMove(BLACK);

    vector<shared_ptr<AbstractPiece>> pieceV;

    pieceV.emplace_back(make_shared<King>(KING, 0, Square(3, 1), ks));
    pieceV.emplace_back(make_shared<King>(KING, 1, Square(6, 1), ks));

    pieceV.emplace_back(make_shared<Queen>(QUEEN, 1, Square(8, 2), ks));
    pieceV.emplace_back(make_shared<Bishop>(BISHOP, 0, Square(4, 4), ks));
    pieceV.emplace_back(make_shared<Bishop>(BISHOP, 1, Square(1, 6), ks));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 1, Square(2, 3), ks, 1));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 1, Square(6, 6), ks, 3));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 0, Square(6, 5), ks, 5));

    for (const shared_ptr<AbstractPiece> &p : pieceV) {
        ks->insertPiece(p);
    }
    ks->updateAllPieces();

    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(1)) {
        string check = a->toString();
        if (a->toString() == "qc2") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;

    EXPECT_EQ(newState.rewards[0], -1);
    EXPECT_EQ(newState.rewards[1], 1);
}

TEST(Kriegspiel, piercingProtection) {
    domains::KriegspielDomain d(4, 4, BOARD::STANDARD);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());
    ks->clearBoard();
    ks->setPlayerOnMove(1);

    vector<shared_ptr<AbstractPiece>> pieceV;

    pieceV.emplace_back(make_shared<King>(KING, 0, Square(8, 8), ks));
    pieceV.emplace_back(make_shared<King>(KING, 1, Square(5, 8), ks));

    pieceV.emplace_back(make_shared<Rook>(ROOK, 1, Square(6, 6), ks));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 0, Square(8, 7), ks, 1));
    pieceV.emplace_back(make_shared<Pawn>(PAWN, 1, Square(4, 4), ks, 3));

    for (shared_ptr<AbstractPiece> p : pieceV) {
        ks->insertPiece(p);
    }
    //move the king to the same square so game doesnt check for castles
    ks[0].getPiecesOfColorAndKind(1, KING)[0]->move(Square(5, 8));
    ks->updateAllPieces();

    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(1)) {
        string check = a->toString();
        if (a->toString() == "kf8") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;

    EXPECT_EQ(newState.rewards[0], 0.0);
    EXPECT_EQ(newState.rewards[1], 0.0);
}

TEST(Kriegspiel, gameOverTest) {
    domains::KriegspielDomain d(4, 4, BOARD::MINIMAL4x3);
    shared_ptr<State> s = d.getRootStatesDistribution()[0].outcome.state;
    auto ks = dynamic_cast<domains::KriegspielState *>(s.get());

    vector<shared_ptr<Action>> v;
    for (shared_ptr<Action> a: ks->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Kb1") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome newState = ks->performActions(v)[0].outcome;
    auto newBoard = dynamic_cast<domains::KriegspielState *>(newState.state.get());

    v.clear();
    for (shared_ptr<Action> a: newBoard->getAvailableActionsFor(1)) {
        string check = a->toString();
        if (a->toString() == "kb3") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome lastState = newBoard->performActions(v)[0].outcome;
    auto lastBoard = dynamic_cast<domains::KriegspielState *>(lastState.state.get());

    v.clear();
    for (shared_ptr<Action> a: lastBoard->getAvailableActionsFor(0)) {
        string check = a->toString();
        if (a->toString() == "Rd3") {
            v.emplace_back(a);
            v.emplace_back(d.getNoAction());
            break;
        }
    }

    Outcome lastsState = lastBoard->performActions(v)[0].outcome;

    EXPECT_EQ(lastsState.rewards[0], 1);
    EXPECT_EQ(lastsState.rewards[1], -1);
}

TEST(Kriegspiel, observationConsistencyOnPawnTakes) {
    domains::KriegspielDomain d1(4, 4, SILVERMAN4BY4);
    domains::KriegspielDomain d2(4, 4, SILVERMAN4BY4);

    shared_ptr<State> root1 = d1.getRootStatesDistribution()[0].outcome.state;
    shared_ptr<State> root2 = d2.getRootStatesDistribution()[0].outcome.state;
    auto state1 = dynamic_cast<domains::KriegspielState *>(root1.get());
    auto state2 = dynamic_cast<domains::KriegspielState *>(root2.get());

    //axb3 e.g. pawn on a2 captures on b3
    auto a1 = make_shared<KriegspielAction>(0, pair<shared_ptr<AbstractPiece>,
                                                    Square>(state1->getPieceOnCoords(Square(1, 2)),
                                                            Square(2, 3)), Square(1, 2));
    //cxb3 e.g. pawn on c2 captures on b3
    auto a2 = make_shared<KriegspielAction>(0, pair<shared_ptr<AbstractPiece>,
                                                    Square>(state1->getPieceOnCoords(Square(3, 2)),
                                                            Square(2, 3)), Square(3, 2));

    vector<shared_ptr<Action>> actions1;
    actions1.emplace_back(a1);
    actions1.emplace_back(d1.getNoAction());

    vector<shared_ptr<Action>> actions2;
    actions2.emplace_back(a2);
    actions2.emplace_back(d2.getNoAction());

    Outcome outcome1 = state1->performActions(actions1)[0].outcome;
    Outcome outcome2 = state2->performActions(actions2)[0].outcome;

    EXPECT_EQ(outcome1.publicObservation->getId(), outcome2.publicObservation->getId());
    EXPECT_EQ(outcome1.publicObservation->getId(), 536870922);
}

TEST(Kriegspiel, observationConsistencyChecks) {
    domains::KriegspielDomain domain(4, 4, BOARD::STANDARD);
    shared_ptr<State> rootState = domain.getRootStatesDistribution()[0].outcome.state;
    auto state = dynamic_cast<domains::KriegspielState *>(rootState.get());
    state->clearBoard();
    //built model situation
    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, WHITE, Square(1, 1), state);
    shared_ptr<King> blackKing = make_shared<King>(KING, BLACK, Square(3, 6), state);

    shared_ptr<Queen> whiteQueen = make_shared<Queen>(QUEEN, WHITE, Square(4, 4), state);
    shared_ptr<Knight> whiteKnight = make_shared<Knight>(KNIGHT, WHITE, Square(1, 2), state);
    state->insertPiece(whiteKing);
    state->insertPiece(blackKing);
    state->insertPiece(whiteQueen);
    state->insertPiece(whiteKnight);
    state->updateAllPieces();

    vector<shared_ptr<Action>> actions;
    auto shortDiagCheck = make_shared<KriegspielAction>(
        0, pair<shared_ptr<AbstractPiece>, Square>(
            state->getPieceOnCoords(Square(4, 4)), Square(1, 4)), Square(4, 4));

    auto longDiagCheck = make_shared<KriegspielAction>(
        0, pair<shared_ptr<AbstractPiece>, Square>(
            state->getPieceOnCoords(Square(4, 4)), Square(5, 4)), Square(4, 4));

    auto verticalCheck = make_shared<KriegspielAction>(
        0, pair<shared_ptr<AbstractPiece>, Square>(
            state->getPieceOnCoords(Square(4, 4)), Square(3, 4)), Square(4, 4));

    auto horizontalCheck = make_shared<KriegspielAction>(
        0, pair<shared_ptr<AbstractPiece>, Square>(
            state->getPieceOnCoords(Square(4, 4)), Square(6, 6)), Square(4, 4));

    auto knightCheck = make_shared<KriegspielAction>(
        0, pair<shared_ptr<AbstractPiece>, Square>(
            state->getPieceOnCoords(Square(1, 2)), Square(2, 4)), Square(1, 2));

    actions.emplace_back(verticalCheck);
    actions.emplace_back(horizontalCheck);
    actions.emplace_back(longDiagCheck);
    actions.emplace_back(shortDiagCheck);
    actions.emplace_back(knightCheck);

    vector<ObservationId> expectedIds{2147483648, 1073741824, 536870912, 268435456, 134217728};
    for (int i = 0; i < 5; i++) {
        vector<shared_ptr<Action>> act;
        act.emplace_back(actions[i]);
        act.emplace_back(domain.getNoAction());
        Outcome outcome = state->performActions(act)[0].outcome;
//        cout << outcome.state->toString() << endl;
        EXPECT_EQ(outcome.publicObservation->getId(), expectedIds[i]);
    }
}

TEST(Kriegspiel, observationConsistencyPawnPieceCapture) {
    domains::KriegspielDomain domain(4, 4, BOARD::STANDARD);
    shared_ptr<State> rootState = domain.getRootStatesDistribution()[0].outcome.state;
    auto state = dynamic_cast<domains::KriegspielState *>(rootState.get());
    state->clearBoard();

    //need kings on board for the game to function
    shared_ptr<King> whiteKing = make_shared<King>(KING, WHITE, Square(1, 1), state);
    shared_ptr<King> blackKing = make_shared<King>(KING, BLACK, Square(3, 1), state);

    shared_ptr<Queen> whiteQueen = make_shared<Queen>(QUEEN, WHITE, Square(4, 4), state);
    shared_ptr<Pawn> blackPawn = make_shared<Pawn>(PAWN, BLACK, Square(1, 7), state, 1);
    shared_ptr<Knight> blackKnight = make_shared<Knight>(KNIGHT, BLACK, Square(4, 8), state);
    state->insertPiece(whiteKing);
    state->insertPiece(blackKing);
    state->insertPiece(whiteQueen);
    state->insertPiece(blackPawn);
    state->insertPiece(blackKnight);
    state->updateAllPieces();
    // capturing actions
    auto takePawn = make_shared<KriegspielAction>(0, pair<shared_ptr<AbstractPiece>, Square>(
        state->getPieceOnCoords(Square(4, 4)), Square(1, 7)), Square(4, 4));
    auto takeKnight = make_shared<KriegspielAction>(0, pair<shared_ptr<AbstractPiece>, Square>(
        state->getPieceOnCoords(Square(4, 4)), Square(4, 8)), Square(4, 4));

    vector<shared_ptr<Action>> actions;

    //capture Knight
    actions.emplace_back(takeKnight);
    actions.emplace_back(domain.getNoAction());
    Outcome outcomeKnight = state->performActions(actions)[0].outcome;
    EXPECT_EQ(outcomeKnight.publicObservation->getId(), 124);

    actions.clear();
//    capture pawn
    actions.emplace_back(takePawn);
    actions.emplace_back(domain.getNoAction());
    Outcome outcomePawn = state->performActions(actions)[0].outcome;
    EXPECT_EQ(outcomePawn.publicObservation->getId(), 49);
}

TEST(Kriegspiel, checkWithMultiplePieces) {
    domains::KriegspielDomain domain(4, 4, BOARD::STANDARD);
    shared_ptr<State> rootState = domain.getRootStatesDistribution()[0].outcome.state;
    auto state = dynamic_cast<domains::KriegspielState *>(rootState.get());
    state->clearBoard();

    shared_ptr<King> whiteKing = make_shared<King>(KING, WHITE, Square(8, 1), state);
    shared_ptr<King> blackKing = make_shared<King>(KING, BLACK, Square(3, 7), state);

    shared_ptr<Queen> whiteQueen = make_shared<Queen>(QUEEN, WHITE, Square(7, 3), state);
    shared_ptr<Knight> whiteKnight = make_shared<Knight>(KNIGHT, WHITE, Square(6, 4), state);
    shared_ptr<Bishop> blackBishop = make_shared<Bishop>(BISHOP, BLACK, Square(6, 6), state);

    state->insertPiece(whiteKing);
    state->insertPiece(blackKing);
    state->insertPiece(whiteQueen);
    state->insertPiece(whiteKnight);
    state->insertPiece(blackBishop);
    state->updateAllPieces();

    vector<shared_ptr<Action>> actions;

    auto knightCheck = make_shared<KriegspielAction>(0, pair<shared_ptr<AbstractPiece>, Square>(
        state->getPieceOnCoords(Square(6, 4)), Square(5, 6)), Square(6, 4));

    actions.emplace_back(knightCheck);
    actions.emplace_back(domain.getNoAction());

    Outcome outcome = state->performActions(actions)[0].outcome;
    EXPECT_EQ(outcome.publicObservation->getId(), 671088640);
}
TEST(Kriegspiel, diagonalChecksObservations) {
    domains::KriegspielDomain domain(4, 4, BOARD::STANDARD);
    shared_ptr<State> rootState = domain.getRootStatesDistribution()[0].outcome.state;
    auto state = dynamic_cast<domains::KriegspielState *>(rootState.get());
    state->clearBoard();

    vector<shared_ptr<Action>> actions;
    auto noAction = domain.getNoAction();

    shared_ptr<King> whiteKing = make_shared<King>(KING, WHITE, Square(8, 1), state);
    shared_ptr<King> blackKing = make_shared<King>(KING, BLACK, Square(3, 5), state);
    shared_ptr<Bishop> whiteBishop = make_shared<Bishop>(BISHOP, WHITE, Square(3, 1), state);
    shared_ptr<Bishop> whiteBishop2 = make_shared<Bishop>(BISHOP, WHITE, Square(3, 7), state);

    state->insertPiece(whiteKing);
    state->insertPiece(blackKing);
    state->insertPiece(whiteBishop);
    state->insertPiece(whiteBishop2);
    state->updateAllPieces();

    vector<Square> movePosition{Square(5, 3), Square(1, 3), Square(4, 6), Square(2, 6)};
    vector<ObservationId> expectedObservations{536870912, 268435456, 268435456, 536870912};
    vector<Square> bishopPos{Square(3, 1), Square(3, 7)};

    for (int i = 0; i < movePosition.size(); ++i) {
        int bishop = i < 2 ? 0 : 1;
        actions.clear();
        auto bishopMove = make_shared<KriegspielAction>(0,
                                                        pair<shared_ptr<AbstractPiece>, Square>(
                                                            state->getPieceOnCoords(bishopPos.at(
                                                                bishop)), movePosition.at(i)),
                                                        bishopPos.at(bishop));
        actions.emplace_back(bishopMove);
        actions.emplace_back(noAction);
        Outcome outcome = state->performActions(actions)[0].outcome;
        EXPECT_EQ(outcome.publicObservation->getId(), expectedObservations.at(i));
    }
}
//TODO write more test on diagonal checks
}