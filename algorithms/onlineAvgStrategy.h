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


#ifndef ALGORITHMS_ONLINEAVGSTRATEGY_H_
#define ALGORITHMS_ONLINEAVGSTRATEGY_H_

#include "algorithms/cfr.h"
#include "algorithms/oos.h"
#include "algorithms/mccr.h"
#include "algorithms/utility.h"
#include "algorithms/common.h"
#include "algorithms/evaluation.h"
#include "cli/cli_helpers.h"

// Compute an offline strategy from an online algorithm

// todo: make a nicer templated version which works with whatever GamePlayingAlgorithm
// todo: refactor to not use CLI:: namespace

namespace GTLib2::algorithms {


StrategyProfile OOS_AverageStrategy(const Domain &domain, const CLI::AlgParams &cfg,
                                    unsigned int preplayBudget, unsigned int moveBudget,
                                    BudgetType budgetType) {
    const auto alg = CLI::constructAlgWithData(domain, "OOS", cfg);
    const unique_ptr<GamePlayingAlgorithm> gameAlg = alg->prepare()(domain, Player(0));
    auto *oos = dynamic_cast<OOSAlgorithm *>(gameAlg.get());
    const auto settings = oos->getSettings();

    LOG_DEBUG("Calculating strategy in the root")
    playForBudget(*gameAlg, PLAY_FROM_ROOT, preplayBudget, budgetType);

    // stupidly handle initial observations by copying the root cache twice for each player
    // these are pointers because EFGCache contains some const fields, and we cannot implement
    // ass operator=()
    auto dataAtISDepth = array<vector<OOSData *>, 2>();
    for (int pl = 0; pl < 2; ++pl) {
        dataAtISDepth.at(pl).reserve(16);
        dataAtISDepth.at(pl).emplace_back(new OOSData(oos->getCache()));
    }

    auto depth = array<unsigned int, 2>{0, 0};
    OOSData targetData = oos->getCache();
    targetData.buildTree();

    LOG_DEBUG("Exploitability after preplay")
    calcExploitability(domain, getAverageStrategy(targetData));

    function<void(shared_ptr<EFGNode>)> traverse = [&](const shared_ptr<EFGNode> &node) -> void {
        if (node->type_ == PlayerNode) {
            const auto infoset = node->getAOHInfSet();
            const auto pl = node->getPlayer();
            auto &playerData = dataAtISDepth.at(pl);
            auto &d = depth.at(pl);
            LOG_DEBUG("Play at infoset " << *infoset)

            playerData.emplace(
                playerData.begin() + (d + 1),
                new OOSData(*playerData.at(d)));

            auto oosAlg = OOSAlgorithm(domain, pl, *playerData.at(d + 1), settings);
            playForBudget(oosAlg, node->getAOHInfSet(), moveBudget, budgetType);

            // copy to evaluated strategy
            if (playerData.at(d + 1)->infosetData.find(infoset)
                != playerData.at(d + 1)->infosetData.end()) {
                targetData.infosetData.at(infoset) = playerData.at(d + 1)->infosetData.at(infoset);
            }

            d++;
        }

        if (node->type_ != TerminalNode) {
            auto actions = node->availableActions();
            for (EdgeId i = 0; i < node->countAvailableActions(); ++i) {
                const auto nextNode = node->performAction(actions.at(i));
                traverse(nextNode);
            }
        }

        if (node->type_ == PlayerNode) {
            const auto pl = node->getPlayer();
            auto &playerData = dataAtISDepth.at(pl);
            // we use pointers instead of unique_ptr
            // because those would be destructed when the entire recursion goes out of scope
            delete playerData.at(depth.at(pl));
            depth.at(pl)--;
        }
    };

    if(moveBudget > 0) {
        traverse(createRootEFGNode(domain));
    }

    return getAverageStrategy(targetData);
}

BehavioralStrategy MCCR_AverageStrategyForPlayer(Player traversingPlayer,
                                                 MCCRAlgorithm *mccr,
                                                 const Domain &domain,
                                                 const CLI::AlgParams &,
                                                 unsigned int moveBudget,
                                                 BudgetType budgetType) {

    const auto settings = mccr->getSettings();

    // Player data at each depth of the public tree. This works as a "strategy" stack.
    auto playerData = vector<MCCRData *>();
    // The strategy at index 0 is the one from pre-play.
    // i.e. public tree root node has been done in preplay.
    playerData.emplace_back(new MCCRData(mccr->getCache())); // a copy!

    // this is the storage for our final strategy, collected individually at each public state
    MCCRData targetData = mccr->getCache(); // a copy!
    targetData.buildTree();

    unsigned int d = 0;

    function<void(shared_ptr<PublicState>)> traverse = [&](const shared_ptr<PublicState> &node) {
//        if (psIsTerminal(targetData, node)) {
//            LOG_DEBUG("Reached terminal public state " << *node)
//            return;
//        }

        d++;
        LOG_DEBUG("Play at public state " << *node << " at depth " << d)

        // copy data at previous depth to the current one
        assert(playerData.size() == d);
        playerData.emplace_back(new MCCRData(*playerData.at(d - 1)));
        MCCRData &currentData = *playerData.at(d);

        // check if player has any infosets in which he can play at the public state
        bool playerIsOwnerOfSomeIS = false;
        shared_ptr<AOH> anInfoset;
        if(currentData.hasPublicState(node)) {
            const auto& efgNodes = currentData.getNodesForPubState(node);
            for (const auto&efgNode : efgNodes) {
                if(efgNode->type_ == PlayerNode && efgNode->getPlayer() == traversingPlayer)  {
                    playerIsOwnerOfSomeIS = true;
                    anInfoset = efgNode->getAOHInfSet();
                    break;
                }
            }
        }

        // This can be actually problematic in complicated public states.
        // The player may not have sampled his infoset in the public state,
        // and we will not improve his strategy there - we'll let it be random.
        if (playerIsOwnerOfSomeIS) {
            auto alg = MCCRAlgorithm(domain, traversingPlayer, currentData, settings);
            // technically, we should do this for each infoset individually
            // because mccr can bias sampling towards the current infoset in the public state.
            // However, for practical purposes, it would be super expensive to do so,
            // as we'd have to re-calculate strategy for each infoset in the tree!
            // Currently, this evaluation takes ~1 day for ~100k sampling per move,
            // to do this per each infoset would be ~100x times more expensive.
            playForBudget(alg, anInfoset, moveBudget, budgetType);

            // copy to evaluated strategy
            for (const auto &infoset : targetData.getInfosetsForPubStatePlayer(
                node, traversingPlayer)) {
                if (currentData.infosetData.find(infoset) != currentData.infosetData.end()) {
                    targetData.infosetData.at(infoset) = currentData.infosetData.at(infoset);
                }
            }
        } else {
            // This might reset oponent strategy. That's fine, because we will call
            // the computation for him separately.
            for (const auto &infoset : targetData.getInfosetsForPubStatePlayer(
                node, traversingPlayer)) {
                // could be augmented IS
                if (targetData.infosetData.find(infoset) != targetData.infosetData.end()) {
                    targetData.infosetData.at(infoset).reset(); // play uniformly random
                }
            }
        }

        for (EdgeId i = 0; i < cntPsChildren(targetData, node); ++i) {
            const auto nextNode = expandPs(targetData, node, i);
            traverse(nextNode);
        }

        // we use pointers instead of unique_ptr
        // because those would be destructed when the entire recursion goes out of scope
        delete playerData.at(d);
        playerData.pop_back();

        d--;
    };

    // root node has been done in preplay
    auto rootNode = targetData.getRootPublicState();
    for (EdgeId i = 0; i < cntPsChildren(targetData, rootNode); ++i) {
        const auto nextNode = expandPs(targetData, rootNode, i);
        traverse(nextNode);
    }

    auto avgStratForPlayer = getAverageStrategy(targetData);
//    auto expl_partial = calcExploitability(domain, avgStratForPlayer);

//    LOG_INFO("Exploitability after resolving for player" << int(traversingPlayer))
//    LOG_VAR(expl_partial.expl)
    return avgStratForPlayer.at(traversingPlayer);
}

StrategyProfile MCCR_AverageStrategy(const Domain &domain, const CLI::AlgParams &cfg,
                                     unsigned int preplayBudget, unsigned int moveBudget,
                                     BudgetType budgetType) {
    const auto alg = CLI::constructAlgWithData(domain, "MCCR", cfg);
    const unique_ptr<GamePlayingAlgorithm> gameAlg = alg->prepare()(domain, Player(0));
    auto *mccr = dynamic_cast<MCCRAlgorithm *>(gameAlg.get());

    LOG_DEBUG("Calculating strategy in the root")
    playForBudget(*gameAlg, PLAY_FROM_ROOT, preplayBudget, budgetType);

//    MCCRData preplayData = mccr->getCache(); // notice we copy!
    // let's build the whole tree if there are missing places where the alg didn't do it
    // during the preplay phase. We need this to calc exploitability.
//    preplayData.buildTree();
//    LOG_INFO("Exploitability after preplay")
//    calcExploitability(domain, getAverageStrategy(preplayData));

    StrategyProfile profile;
    for (Player traversingPlayer = 0; traversingPlayer <= 1; ++traversingPlayer) {
        LOG_DEBUG("Calculating strategy for player " << int(traversingPlayer))
        profile.emplace_back(MCCR_AverageStrategyForPlayer(
            traversingPlayer, mccr, domain, cfg, moveBudget, budgetType));
    }

    return profile;
}

constexpr int DBAR_NUM_SEEDS = 50;

StrategyProfile MCCR_DbarAverageStrategy(const Domain &domain, const CLI::AlgParams &cfg,
                                         unsigned int preplayBudget, unsigned int moveBudget,
                                         BudgetType budgetType) {

    vector<StrategyProfile> profiles;
    for (int i = 0; i < DBAR_NUM_SEEDS; ++i) {
        LOG_INFO("Calculating seed " << i)
        CLI::AlgParams cfg_seed = cfg;
        cfg_seed.emplace("seed", to_string(i));
        profiles.emplace_back(
            MCCR_AverageStrategy(domain, cfg_seed, preplayBudget, moveBudget, budgetType));
    }

    // calc avg over individual seeds
    StrategyProfile profile = profiles.at(0);
    for (int p = 0; p < 2; ++p) {
        for(auto&[infoset, probs] : profile.at(p)) {
            for (auto&[action, prob]: probs) {
                for (int i = 1; i < DBAR_NUM_SEEDS; ++i) {
                    const auto &profile_seed = profiles[i];
                    const ActionProbDistribution &probs_seed = profile_seed.at(p).at(infoset);
                    prob += probs_seed.at(action);
                }
                prob /= DBAR_NUM_SEEDS;
            }
        }
    }

    return profile;
}

}

#endif
