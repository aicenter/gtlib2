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


#ifndef BASE_HISTORY_CONSTRAINTS_H_
#define BASE_HISTORY_CONSTRAINTS_H_

#include "base/algorithm.h"

// This file contains interfaces for domains that are needed
// for additional generation of histories in infosets.
//
// It is motivated by the following scenario:
//
// Suppose we use an online sampling algorithm in a match (like IS-MCTS).
// During the course of the play, our opponent might play an action that leads
// us into an infoset (called target infoset), where we do not have many histories sampled.
//
// We'd like to generate more histories in this target infoset to be able to compute better strategy.
// However, randomly sampling from the root is unlikely to lead us to this target infoset.
//
// We need to employ some algorithm that can find additional histories consistently and fast.
// We formulate this problem as a CSP, and use domain dependent constraints as an input to
// this solver.

namespace GTLib2 {

// Contains any type of constraints that can be described by the domain.
//
// This is different from observations: it interprets how observations map
// into what we can deduce about the opponent. For example, in IIGoofspiel,
// if we draw in a round, it means opponent must have played the same card as we did.
//
// There are two kinds of approaches to constraint generation:
// - by enumeration (of all possible variable values)
//   Goofspiel: List all possible cards at given turn. When we learn something new,
//              we cross out impossible cards at all turns where we can.
// - by elimination (and then enumeration)
//   Stratego:  When we learn something about opponent's figure, we backtrack to
//              the setup action of given cell from which opponent's figure started.
//              We store that the figure was movable, and possibly its rank.
//              This eliminates possible actions and we only enumerate them subsequently.
struct Constraint {
    Constraint() = default;
    virtual ~Constraint() = default;
};

// A map of "turn numbers" and constraints that apply at those "turns" (defined below).
//
// We know exactly what actions we have done in the game to come to the target infoset.
// However, we need to capture what are all the possibilities how the opponent could have come
// there as well.
//
// We do this by maintaing a set of constraints that apply at his "turns".
// A **turn** is player's AOH at opponent's histories (so player's augmented infoset).
// A **turn number** is the size of this AOH. Basically, whenever player receives a new observation
// (his AOH changes), we'd like to limit the choices of opponent's actions based
// on this new observation.
//
// Note that histories with the same turn number are not necessarily at the same EFG tree depth:
// opponent may have done repeated phantom moves, during which the player has not received
// any observations.
//
// If there is no entry for some encountered turn numbers, it means there is no known constraint
// for that turn (the player did not receive any observation relevant for this turn).
//
// Note: This map contains info about the turn when it should apply, not when player learns it!
typedef unordered_map<unsigned long, shared_ptr<Constraint>> ConstraintsMap;

// An interface that domain must implement so we can collect it's constraints.
class ConstrainingDomain {
 public:

    // Initializes enumerative constraints before the start of the game.
    // See Constraint struct for more details.
    virtual void initializeEnumerativeConstraints(ConstraintsMap &constraints) const = 0;

    // Update constraint map when target infoset changes.
    // This will update all constraints after specified turn number.
    //
    // Returns whether any constraint has been updated.
    virtual bool updateConstraints(const shared_ptr<AOH> &targetInfoset,
                                   long &startAtTurnNumber,
                                   ConstraintsMap &constraints) const = 0;

    // Generate new nodes by applying the constraints.
    //
    // The generated nodes may have been already known to the player:
    // it would be technically too burdensome to prohibit the generation of discovered histories:
    // it would add additional constraints to the solver that do not prune the state-space
    // efficiently (they would prune it only for those given histories).
    //
    // In the online scenario (described above) we'd like to generate as many histories as possible
    // given some time budget. It can be done by calling this function with parameter BudgetType = BudgetTime, and
    // budget equal to time in ms. Otherwise budget is the number of nodes to be generated.
    virtual void generateNodes(const shared_ptr<AOH> &targetInfoset,
                               const ConstraintsMap &constraints,
                               BudgetType budgetType,
                               int budget,
                               const EFGNodeCallback &maybeNewNodeCallback) const = 0;
};


typedef function<void(const ConstrainingDomain &domain,
                      const shared_ptr<AOH> &targetInfoset,
                      const BudgetType budgetType,
                      const int budget,
                      const EFGNodeCallback &maybeNewNodeCallback)> EFGNodeGenerator;


inline void emptyNodeGenerator(const ConstrainingDomain &domain,
                               const shared_ptr<AOH> &targetInfoset,
                               const BudgetType budgetType,
                               const int budget,
                               const EFGNodeCallback &maybeNewNodeCallback) {

}

inline void domainSpecificNodeGenerator(const ConstrainingDomain &domain,
                                        const shared_ptr<AOH> &targetInfoset,
                                        const BudgetType budgetType,
                                        const int budget,
                                        const EFGNodeCallback &maybeNewNodeCallback) {
    ConstraintsMap constraints;
    long start = -1;
    domain.initializeEnumerativeConstraints(constraints);

    // checks is the game at current state lets us generate node
    // for example, not valid for stratego during the setup state
    bool validState = domain.updateConstraints(targetInfoset, start, constraints);
    if (validState)
        domain.generateNodes(targetInfoset, constraints,
                             budgetType, budget, maybeNewNodeCallback);
}

inline void cspNodeGenerator(const ConstrainingDomain &domain,
                             const shared_ptr<AOH> &targetInfoset,
                             BudgetType budgetType,
                             int budget,
                             const EFGNodeCallback &maybeNewNodeCallback) {
    unreachable("cspNodeGenerator is not implemented yet");
}

}
#endif  // BASE_HISTORY_CONSTRAINTS_H_
