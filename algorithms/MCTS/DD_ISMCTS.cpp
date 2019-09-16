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

#include "DD_ISMCTS.h"
namespace GTLib2::algorithms {

unsigned int backtrackPosition(const vector<ActionObservationIds> & aoids, unsigned int endpos, unsigned long  i) {
    auto startpos = endpos;
    while(i > 0)
    {
        i--;
        const auto currObs = domains::decodeObservation(aoids[i].observation);
        if (currObs.size() == 2) break;
        if (currObs[1] == startpos) {
            if (currObs[3] != domains::EMPTY && !domains::isFigureSlain(currObs[2], currObs[3]))
                continue;
            startpos = currObs[0];
        }
    }
    return startpos;
}

PlayControl DD_ISMCTS::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    if (currentInfoset == PLAY_FROM_ROOT)  {
        iteration(rootNode_);
        return ContinueImproving;
    }

    if (currentInfoset_ != *currentInfoset) {
        setCurrentInfoset(*currentInfoset);
    }

    const auto aoids = currentInfoset_->getAOids();
    if (aoids.size() > 2 && !currentISChecked_)
    {
        bool newFigureRevealed = false;
        for (unsigned long i = lastRevealAoid_ + 1; i < aoids.size(); i++)
        {
            if (aoids[i].observation == domain_.getNoObservation()->getId()) break;
            const auto res = domains::decodeObservation(aoids[i].observation);
            if (res.size() == 4 && res[3] != domains::EMPTY)
            {
                unsigned long pos = backtrackPosition(aoids, (aoids[i].action == domain_.getNoAction()->getId()) ? res[0] : res[1], i);
                const auto stratDomain = dynamic_cast<const domains::StrategoDomain&>(domain_);
                if (pos > stratDomain.startFigures_.size()) pos = stratDomain.boardHeight_*stratDomain.boardWidth_ - pos - 1;
                const domains::Rank val = domains::getRank((aoids[i].action == domain_.getNoAction()->getId()) ? res[2] : res[3]);
                if (revealedFigures_[pos] == domains::EMPTY) revealedCounter_++;
                else if (revealedFigures_[pos] != val)
                    unreachable("Incorrect revealing");
                revealedFigures_[pos] = val;
                lastRevealAoid_ = i;
                newFigureRevealed = true;
            }
        }
        if (newFigureRevealed)
            generate();
        currentISChecked_ = true;
    }

    if (infosetSelectors_.find(currentInfoset_) == infosetSelectors_.end())
    {
        isCurrentISUndiscovered_ = true;
        if (iterateRoot_) iteration(rootNode_);
        return ContinueImproving;
    }

    const auto nodes = nodesMap_.find(currentInfoset_);
    if (nodes == nodesMap_.end())
    {
        isCurrentISUndiscovered_ = true;
        if (iterateRoot_) iteration(rootNode_);
        return ContinueImproving;
    }

    isCurrentISUndiscovered_ = false;
    const int nodeIndex = pickRandomInt(0, nodes->second.size() - 1, generator_);
    iteration(nodes->second[nodeIndex]);
    return ContinueImproving;
}

optional<ProbDistribution> DD_ISMCTS::getPlayDistribution(const shared_ptr<AOH> &currentInfoset,
                                                            const long actionsNum) {
    if (isCurrentISUndiscovered_)
    {
        auto v = vector<double>(actionsNum);
        std::fill(v.begin(), v.end(), 1.0/actionsNum);
        return v;
    }
    return ISMCTS::getPlayDistribution(currentInfoset, actionsNum);
}

void DD_ISMCTS::generate()
{
    const auto stratDomain = dynamic_cast<const domains::StrategoDomain&>(domain_);
    vector<domains::Rank> newFigures;
    vector<domains::Rank> revealedFigures = revealedFigures_;
    const auto aoids = currentInfoset_->getAOids();
    for (unsigned long i = 0; i < revealedFigures_.size(); i++)
    {
        bool figureFound = false;
        for (unsigned long j = 0; j < revealedFigures_.size(); j++)
        {
            if (revealedFigures[j] != domains::EMPTY && stratDomain.startFigures_[i] == revealedFigures[j])
            {
                figureFound = true;
                revealedFigures[j] = domains::EMPTY;
                break;
            }
        }
        if (!figureFound)
        {
            newFigures.push_back(stratDomain.startFigures_[i]);
        }
    }
    unsigned long max = domains::countDistinctPermutations(newFigures);
    if (max > generateIters_) max = generateIters_;
    if (selectedPermutation_ == nullptr) selectedPermutation_ = rootNode_->getActionByID(aoids[1].action);
    unsigned long permutationIndex = 0;
    auto currentPermutation = stratDomain.startFigures_;
    for (unsigned long i = 0; i < max; i++)
    {
        vector<domains::Rank> t = newFigures;//domains::permutations(newFigures, i);
        std::next_permutation(newFigures.begin(), newFigures.end());
        for (unsigned long j = 0; j < revealedFigures_.size(); j++)
        {
            if (revealedFigures_[j] != domains::EMPTY)
            {
                t.insert(t.begin() + j, revealedFigures_[j]);
            }
        }

        auto currNode = rootNode_;
        if (playingPlayer_ == 0)
            currNode = currNode->performAction(selectedPermutation_);
        bool setupActionFound = false;
        do {
            if (currentPermutation == t)
            {
                auto a = make_shared<domains::StrategoSetupAction>(permutationIndex, t);
                currNode = currNode->performAction(a);
                setupActionFound = true;
                break;
            }
            permutationIndex++;
        } while (next_permutation(currentPermutation.begin(), currentPermutation.end()));
        assert(setupActionFound); // debug
        assert(permutationIndex < domains::countDistinctPermutations(stratDomain.startFigures_)); // debug
        if (playingPlayer_ == 1)
            currNode = currNode->performAction(selectedPermutation_);
        for (unsigned long j = playingPlayer_ == 0 ? 3 : 2; j < aoids.size(); j++)
        {
            if (*currNode->getAOHInfSet() == *currentInfoset_)
            {
                iteration(currNode);
                isCurrentISUndiscovered_ = false;
                break;
            }
            if (aoids[j].action == stratDomain.getNoAction()->getId())
            {
                if (aoids[j].observation == OBSERVATION_PLAYER_MOVE)
                    unreachable("Node generation failed"); // debug
                const auto currObs = domains::decodeObservation(aoids[j].observation);
                bool moveActionFound = false;
                for (const auto& a : currNode->availableActions())
                {
                    const auto action = dynamic_pointer_cast<domains::StrategoMoveAction>(a);
                    if (action->startPos == currObs[0] && action->endPos == currObs[1])
                    {
                        moveActionFound = true;
                        currNode = currNode->performAction(a);
                        break;
                    }
                }
                if (!moveActionFound)
                    break; //same action not found => board setup does not fit (for example, bomb in place of movable piece)
            }
            else
                currNode = currNode->performAction(currNode->getActionByID(aoids[j].action));
        }
    }
}

void DD_ISMCTS::setCurrentInfoset(const shared_ptr<AOH> &newInfoset) {
    currentInfoset_ = newInfoset;
    currentISChecked_ = false;
}
}