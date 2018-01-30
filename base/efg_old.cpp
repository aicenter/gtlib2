//
// Created by Pavel Rytir on 1/20/18.
//

#include "efg_old.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"


namespace GTLib2 {

    unordered_map<size_t, vector<EFGNode>> mapa;

    extern int countStates;  // temporary for testing treewalk

    vector<double> reward;  // temporary for testing treewalk TODO: remove it

    void OldEFGTreewalkStart(const shared_ptr<Domain> &domain,
                             std::function<void(EFGNode *)> FunctionForState,
                             unsigned int depth) {
        if (depth == 0)
            depth = domain->getMaxDepth();
        auto node = MakeUnique<EFGNode>();
        ChanceNode chan(domain->getRootStateDistributionPtr().get(), {}, node);
        vector<unique_ptr<EFGNode>> vec = chan.GetAll();
        for (auto &j : vec) {
            ++countStates;
            reward += j->GetRewards();
            OldEFGTreewalk(domain, j.get(), depth, 1, {}, FunctionForState);
        }
    }

    void OldEFGTreewalkStart(const shared_ptr<Domain> &domain, unsigned int depth) {
        OldEFGTreewalkStart(domain, [](EFGNode *s) {}, depth);
    }


    void OldEFGTreewalk(const shared_ptr<Domain> &domain, EFGNode *node,
                        unsigned int depth, int players,
                        const vector<shared_ptr<Action>> &list,
                        std::function<void(EFGNode *)> FunctionForState) {
        if (node == nullptr) {
            throw ("Node is NULL");
        }

        if (depth == 0) {
            return;
        }

        FunctionForState(node);

        vector<shared_ptr<Action>> actions = node->GetAction();

        auto search = mapa.find(node->GetIS()->getHash());
        if (search != mapa.end()) {
            search->second.emplace_back(node->GetPlayer(), node->getState(),
                                        node->GetRewards(), node->GetParent(),
                                        node->GetLast());
        } else {
            auto n2 = EFGNode(node->GetPlayer(), node->getState(),
                              node->GetRewards(), node->GetParent(), node->GetLast());
            mapa.emplace(n2.GetIS()->getHash(), vector<EFGNode>{n2});
        }

        for (auto &i : actions) {
            unique_ptr<EFGNode> n = node->OldPerformAction(i);
            vector<shared_ptr<Action>> locallist = list;
            locallist.push_back(i);

            if (players == n->getState()->getNumberOfPlayers()) {
                int actionssize = locallist.size();
                int index = 1;
                while (node->GetPlayer() >= actionssize) {
                    n->PushLast(node->GetLast()[2 * index], node->GetLast()[2 * index + 1]);
                    locallist.insert(locallist.begin(), make_shared<Action>(NoA));
                    ++actionssize;
                    ++index;
                    FunctionForState(n.get());
                }
                while (domain->getNumberOfPlayers() > locallist.size()) {
                    locallist.push_back(make_shared<Action>(NoA));
                    if (!node->GetLast().empty()) {
                        n->PushLast(node->GetLast()[2 * index], node->GetLast()[2 * index + 1]);
                    }
                    ++index;
                    FunctionForState(n.get());
                }
                // if all players play in this turn, returns a ProbDistribution
                OutcomeDistributionOld prob = n->getState()->PerformAction(locallist);
                ChanceNode chan(&prob, locallist, n);
                vector<unique_ptr<EFGNode>> vec = chan.GetAll();
                for (auto &j : vec) {
                    ++countStates;
                    reward += j->GetRewards();
                    OldEFGTreewalk(domain, j.get(), depth - 1, 1, {}, FunctionForState);
                }
            } else {
                if (depth < domain->getMaxDepth()) {
                    int index = n->GetPlayer();
                    while (index < domain->getNumberOfPlayers()) {
                        n->PushLast(node->GetLast()[2 * index], node->GetLast()[2 * index + 1]);
                        ++index;
                    }
                }
                OldEFGTreewalk(domain, n.get(), depth, players + 1, locallist, FunctionForState);
            }
        }
    }

    void OldEFGTreewalk(const shared_ptr<Domain> &domain, EFGNode *node,
                        unsigned int depth, int players,
                        const vector<shared_ptr<Action>> &list) {
        OldEFGTreewalk(domain, node, depth, players, list, [](EFGNode *s) {});
    }


    ChanceNode::ChanceNode(OutcomeDistributionOld *prob,
                           const vector<shared_ptr<Action>> &actions,
                           const unique_ptr<EFGNode> &node) :
            prob_(prob), actions_(actions), node_(node) {}


    vector<unique_ptr<EFGNode>> ChanceNode::GetAll() {
        vector<Outcome> outcomes = prob_->GetOutcomes();
        vector<unique_ptr<EFGNode>> vec;
        vec.reserve(outcomes.size());
        if (node_->GetPlayer() >= 0) {  // preparing first states
            for (Outcome &o : outcomes) {
                vector<double> rews = vector<double>(o.GetReward().size());
                rews = node_->GetRewards() + o.GetReward();
                auto players = o.GetState()->GetPlayers();
                for (unsigned int i = 0; i < players.size(); ++i) {
                    if (players[i]) {
                        vector<int> aoh{actions_[i]->getId(), o.GetObs()[i]->getId()};
                        for (unsigned int j = 0; j < players.size(); ++j) {
                            if (j == i)
                                continue;
                            aoh.push_back(actions_[j]->getId());
                            aoh.push_back(o.GetObs()[j]->getId());
                        }
                        auto n = MakeUnique<EFGNode>(i, o.GetState(), rews, node_.get(), aoh);
                        vec.push_back(move(n));
                        break;
                    }
                }
            }
            return vec;
        }
        for (Outcome &o : outcomes) {
            vec.push_back(MakeUnique<EFGNode>(0, o.GetState(),
                                              vector<double>(o.GetReward().size()), node_.get()));
        }
        return vec;
    }

}

#pragma clang diagnostic pop