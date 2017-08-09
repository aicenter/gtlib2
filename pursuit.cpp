//
// Created by rozliv on 02.08.2017.
//

#include "pursuit.h"

std::string PursuitAction::ToString() {
    return std::string();
}

PursuitAction::PursuitAction(int id): Action(id) {}

PursuitState::PursuitState(std::vector<Pos> &p):place_(p){
    probdis_ = {0.1,0.9};
}

PursuitState::PursuitState(std::vector<Pos> &p, double prob):place_(p), prob_(prob){
    probdis_ = {0.1,0.9};
}

std::vector<Action> PursuitState::getActions(int player) {
    std::vector<Action> list = std::vector<Action>();
    getActions(list,player);
    return list;
}

void PursuitState::getActions(std::vector<Action> &list,int player)const {
    for (int i = 1; i < 5; ++i) {
        if((place_[player].x + m_[i].x) >= 0 && (place_[player].x + m_[i].x) < PursuitDomain::width_
           &&(place_[player].y + m_[i].y) >= 0 && (place_[player].y + m_[i].y) < PursuitDomain::height_){
            list.emplace_back(i);
        }
    }

}

ProbDistribution PursuitState::PerformAction(std::vector<Action>& actions)const {
    std::vector<Pos> moves = std::vector<Pos>();
    moves.reserve(actions.size());
    auto count = static_cast<unsigned int>(pow(2, actions.size()));
    std::vector<std::shared_ptr<PursuitState>> st;// = std::vector<PursuitState>(count);
    std::vector<double> rew = std::vector<double>();
    std::vector<std::vector<double> > vec;
    std::vector<std::vector<Observation> > obsvec;
    std::vector<std::pair<Outcome, double>> pairs;
    for (int k = 0; k < count; ++k) {
        double probability = prob_;
        for (int i = 0; i < actions.size(); ++i) {
            if (k / static_cast<int>(pow(2, i)) % 2) {
                moves.push_back({m_[actions[i].getID()].y + place_[i].y, m_[actions[i].getID()].x + place_[i].x});
                probability *= probdis_[1];
            } else {
                moves.push_back({place_[i].y, place_[i].x});
                probability *= probdis_[0];
            }
        }
        st.emplace_back(new PursuitState(moves, probability));
        moves.clear();
        rew.push_back(0);
        for (int i = 1; i < st[k]->getPlace().size(); ++i) {
            rew.push_back(0);
            if ((st[k]->place_[0].x == place_[i].x && st[k]->place_[0].y == place_[i].y &&
                 st[k]->place_[i].x == place_[0].x &&
                 st[k]->place_[i].y == place_[0].y) ||
                (st[k]->place_[0].x == st[k]->place_[i].x && st[k]->place_[0].y == st[k]->place_[i].y)) {
                ++rew[0];
                --rew[i];
            }
        }
        vec.push_back(rew);
        rew.clear();
        int index;
        std::vector<Observation> obs = std::vector<Observation>();
        obs.emplace_back(0);
        for(int i = 1; i < st[k]->place_.size(); ++i) {
            index = 0;
            for (int l = 1; l < eight.size(); ++l) {
                if (st[k]->place_[0].x + eight[l].x == st[k]->place_[i].x && st[k]->place_[0].y + eight[l].y == st[k]->place_[i].y) {
                    index = l;
                }
            }
            obs[0] = Observation(index,desc[index]);
            obs.emplace_back(eight.size()-1-index,desc[eight.size()-1-index]);
        }
        obsvec.push_back(obs);
        obs.clear();
        Outcome p = Outcome(st.back(), obsvec.back(), vec.back());
        pairs.emplace_back(p, st.back()->prob_/prob_);
    }
    ProbDistribution prob(pairs);
    return prob;
}

PursuitState::PursuitState() = default;

PursuitDomain::PursuitDomain(int max,const std::shared_ptr<State> &r): Domain(r),maxdepth_(max){}

int PursuitDomain::height_ = 0;
int PursuitDomain::width_ = 0;

std::string PursuitDomain::GetInfo() {
    return "Rozmery pole jsou: " + std::to_string(PursuitDomain::height_) + " x " +
           std::to_string(PursuitDomain::height_) + " maximalni hloubka grafu je: " +  std::to_string(maxdepth_) +
           " a pocatecni stav je: ...";
}

int count = 0;
std::vector<double> rewards;

void pursuit( const Domain& domain, std::shared_ptr<State>state, int depth) {
//   for (int i = 0; i < domain.getMaxDepth() - depth; ++i) {
//        cout << "    ";
//    }

//    string s = std::to_string(state.getPlace()[0].x) + " " + std::to_string(state.getPlace()[0].y) + "    " +
//               std::to_string(state.getPlace()[1].x) + " " + std::to_string(state.getPlace()[1].y) + "   " +
//               std::to_string(state.getPro());
    //  cout << s<< '\n';
 //   cout << state.use_count()<<'\n';
    vector<Action> actions = vector<Action>();
    vector<Action> actions2 = vector<Action>();
    ++count;
    if (depth == 0) {
        return;
    }
    state->getActions(actions, 0);
    state->getActions(actions2, 1);

    for (Action &action : actions) {
        for (Action &j : actions2) {
            vector<Action> base = {action, j};
            ProbDistribution prob = state->PerformAction(base);
            for (Outcome &o : prob.GetOutcomes()) {
                for (int i = 0; i < rewards.size(); ++i) {
                    rewards[i] += o.getReward()[i];
                }
                pursuit(domain, o.getState(), depth - 1);
            }
        }
    }
}
