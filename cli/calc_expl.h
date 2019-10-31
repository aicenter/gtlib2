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

#ifndef GTLIB2_EXPERIMENT_CALC_EXPL_H
#define GTLIB2_EXPERIMENT_CALC_EXPL_H

#include "cli_helpers.h"
#include "algorithms/evaluation.h"

namespace GTLib2::CLI {

StrategyProfile CFR_AverageStrategy(const Domain &domain, const AlgParams &cfg,
                                    unsigned int preplayBudget, BudgetType budgetType) {
    const auto alg = constructAlgWithData(domain, "CFR", cfg);
    const unique_ptr<GamePlayingAlgorithm> gameAlg = alg->prepare()(domain, Player(0));
    CFRAlgorithm *cfr = dynamic_cast<CFRAlgorithm *>(gameAlg.get());
    LOG_DEBUG("Calculating strategy")
    playForBudget(*gameAlg, PLAY_FROM_ROOT, preplayBudget, budgetType);
    return getAverageStrategy(cfr->getCache());
}

StrategyProfile OOS_AverageStrategy(const Domain &domain, const AlgParams &cfg,
                                    unsigned int preplayBudget, unsigned int moveBudget,
                                    BudgetType budgetType) {
    const auto alg = constructAlgWithData(domain, "OOS", cfg);
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
    auto expl = calcExploitability(domain, getAverageStrategy(targetData));

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

            auto alg = OOSAlgorithm(domain, pl, *playerData.at(d + 1), settings);
            playForBudget(alg, node->getAOHInfSet(), moveBudget, budgetType);

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
    traverse(createRootEFGNode(domain));

    return getAverageStrategy(targetData);
}

BehavioralStrategy MCCR_AverageStrategyForPlayer(Player traversingPlayer,
                                                 MCCRAlgorithm *mccr,
                                                 const Domain &domain,
                                                 const AlgParams &cfg,
                                                 unsigned int moveBudget,
                                                 BudgetType budgetType) {

    const auto settings = mccr->getSettings();

    // Player data at each depth of the public tree. This works as a "strategy" stack.
    auto playerData = vector<OOSData *>();
    playerData.emplace_back(new OOSData(mccr->getCache()));

    // this is the storage for our final strategy, collected individually at each public state
    OOSData targetData = mccr->getCache();
    targetData.buildTree();

    unsigned int d = 0;

    function<void(shared_ptr<PublicState>)> traverse = [&](const shared_ptr<PublicState> &node) {
        if (psIsTerminal(targetData, node)) {
            LOG_DEBUG("Reached terminal public state " << *node)
            return;
        }
        LOG_DEBUG("Play at public state " << *node)

        // copy data at previous depth to the current one
        playerData.emplace(playerData.begin() + (d + 1), new OOSData(*playerData.at(d)));
        OOSData &currentData = *playerData.at(d + 1);

        if (currentData.hasPublicState(node)) {
            auto alg = OOSAlgorithm(domain, traversingPlayer, currentData, settings);
            auto infosets = currentData.getInfosetsForPubStatePlayer(node, traversingPlayer);
            // technically, we should do this for each infoset individually
            // because mccr can bias sampling towards the current infoset in the public state.
            // However, for practical purposes, it would be super expensive to do so,
            // as we'd have to re-calculate strategy for each infoset in the tree!
            // Currently, this evaluation takes ~1 day for ~100k sampling per move,
            // to do this per each infoset would be ~100x times more expensive.
            const shared_ptr<AOH> anInfoset = *infosets.begin(); // todo: random enough? :)
            playForBudget(alg, anInfoset, moveBudget, budgetType);

            // copy to evaluated strategy
            for (const auto &infoset : targetData.getInfosetsForPubStatePlayer(
                node, traversingPlayer)) {
                if (currentData.infosetData.find(infoset) != currentData.infosetData.end()) {
                    targetData.infosetData.at(infoset) = currentData.infosetData.at(infoset);
                }
            }
        } else {
            for (const auto &infoset : targetData.getInfosetsForPubStatePlayer(
                node, traversingPlayer)) {
                // could be augmented IS
                if (targetData.infosetData.find(infoset) != targetData.infosetData.end()) {
                    targetData.infosetData.at(infoset).reset(); // play uniformly random
                }
            }
        }
        d++;

        for (EdgeId i = 0; i < cntPsChildren(targetData, node); ++i) {
            const auto nextNode = expandPs(targetData, node, i);
            traverse(nextNode);
        }

        // we use pointers instead of unique_ptr
        // because those would be destructed when the entire recursion goes out of scope
        delete playerData.at(d);
        d--;
    };

    traverse(targetData.getRootPublicState());
    auto avgStratForPlayer = getAverageStrategy(targetData);
    auto expl_partial = calcExploitability(domain, avgStratForPlayer);

    LOG_INFO("Exploitability after resolving for player" << int(traversingPlayer))
    LOG_VAR(expl_partial.expl)
    return avgStratForPlayer.at(traversingPlayer);
}

StrategyProfile MCCR_AverageStrategy(const Domain &domain, const AlgParams &cfg,
                                     unsigned int preplayBudget, unsigned int moveBudget,
                                     BudgetType budgetType) {
    const auto alg = constructAlgWithData(domain, "MCCR", cfg);
    const unique_ptr<GamePlayingAlgorithm> gameAlg = alg->prepare()(domain, Player(0));
    auto *mccr = dynamic_cast<MCCRAlgorithm *>(gameAlg.get());

    LOG_DEBUG("Calculating strategy in the root")
    playForBudget(*gameAlg, PLAY_FROM_ROOT, preplayBudget, budgetType);

    OOSData preplayData = mccr->getCache();
    // let's build the whole tree if there are missing places where the alg didn't do it
    // during the preplay phase. We need this to calc exploitability.
    preplayData.buildTree();
    LOG_INFO("Exploitability after preplay")
    calcExploitability(domain, getAverageStrategy(preplayData));

    StrategyProfile profile;
    for (Player traversingPlayer = 0; traversingPlayer < 2; ++traversingPlayer) {
        profile.emplace_back(MCCR_AverageStrategyForPlayer(
            traversingPlayer, mccr, domain, cfg, moveBudget, budgetType));
    }

    return profile;
}

void Command_CalcExpl(args::Subparser &parser) {
    args::Group group(parser, "Algorithms to play");
    args::ValueFlag<unsigned int> preplayBudget(group, "ms", "Budget in preplay",
                                                {"preplay"}, 1000);
    args::ValueFlag<unsigned int> moveBudget(group, "ms", "Budget per move", {"move"}, 0);
    args::Group budgetType(group, "Budget type:", args::Group::Validators::Xor);
    args::Flag time(budgetType, "time", "", {"time"});
    args::Flag iterations(budgetType, "iterations", "", {"iterations", "iters"});
    args::Flag printProfile(parser, "printProfile", "Print strategy profile?", {"print"}, false);
    args::Flag dbar(parser, "dbar", "Compute double bar strategy (expectation over average strats)",
                    {"dbar"}, false);
    initializeParser(parser); // always include this line in command

    const auto algs = args::get(args::alg);
    if (algs.size() != 1) {
        LOG_ERROR("Exactly one algorithm must be specified!");
        exit(1);
    }

    const auto cfgs = CLI::algParams;
    const auto domain = constructDomain(args::get(args::domain));
    const unsigned int pb = args::get(preplayBudget);
    const unsigned int mb = args::get(moveBudget);
    const BudgetType btype = time ? BudgetTime : BudgetIterations;
    const auto alg = algs.at(0);
    const auto cfg = cfgs.at(0);

    if (args::get(args::tag_header)) {
        cout << "preplay" << ",";
        cout << "move" << ",";
        cout << "budget" << ",";
        cout << "expl" << endl;
        throw args::Header();
    }
    if (args::get(args::tag)) {
        cout << pb << ",";
        cout << mb << ",";
        cout << (time ? "time" : "iterations") << ",";
    }

    LOG_INFO("Running expl computation on domain: " << domain->getInfo())
    LOG_INFO("Algorithm: " << alg)
    LOG_INFO("Config file: " << cfg)
    LOG_INFO("Preplay: " << pb)
    LOG_INFO("Per move: " << mb)
    LOG_INFO("Budget: " << (time ? "time" : "iterations"))
    LOG_INFO("------------------------")

    StrategyProfile profile;
    if (alg == "CFR") profile = CFR_AverageStrategy(*domain, cfg, pb, btype);
    else if (alg == "OOS") profile = OOS_AverageStrategy(*domain, cfg, pb, mb, btype);
    else if (alg == "MCCR") profile = MCCR_AverageStrategy(*domain, cfg, pb, mb, btype);
    else LOG_ERROR("Algorthim " << alg << "does not have implemented exploitability calculation");

    if (printProfile) {
        for (int pl = 0; pl < 2; ++pl) {
            cout << "Behavioral strategy for player " << pl << endl;
            cout << profile.at(pl) << endl;
            cout << "-----------" << endl;
        }
    }

    LOG_DEBUG("Calculating exploitability")
    cout << calcExploitability(*domain, profile).expl << endl;
}

}

#endif // GTLIB2_EXPERIMENT_CALC_EXPL_H
