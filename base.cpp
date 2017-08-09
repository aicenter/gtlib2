//
// Created by rozliv on 07.08.2017.
//

#include "base.h"

Action::Action(int id): id_(id) {}

std::string Action::ToString() {
    return std::string();
}

//Action::Action(int t_id,  const std::string& t_s): id(t_id), s(t_s){}


Observation::Observation(int id): id_(id){}

Observation::Observation(int id,  const std::string& s):id_(id), s_(s){}


State::State() = default;

std::vector<Action> State::getActions(int player) {
    return std::vector<Action>();
};

void State::getActions(std::vector<Action> &list, int player) const {}

ProbDistribution State::PerformAction(std::vector<Action> &actions) const {
//    std::vector<Pos> moves = std::vector<Pos>();
//    moves.reserve(actions.size());
//    auto count = static_cast<unsigned int>(pow(2, actions.size()));
//    std::vector<PursuitState> st;// = std::vector<PursuitState>(count);
//    std::vector<double> rew = std::vector<double>();
//    std::vector<std::vector<double> > vec;
//    std::vector<std::vector<Observation> > obsvec;
//    std::vector<std::pair<PursuitOutcome, double>> pairs;
//    for (int k = 0; k < count; ++k) {
//        double probability = prob_;
//        for (int i = 0; i < actions.size(); ++i) {
//            if (k / static_cast<int>(pow(2, i)) % 2) {
//                moves.push_back({m_[actions[i].getID()].y + place_[i].y, m_[actions[i].getID()].x + place_[i].x});
//                probability *= probdis_[1];
//            } else {
//                moves.push_back({place_[i].y, place_[i].x});
//                probability *= probdis_[0];
//            }
//        }
//        st.emplace_back(moves, probability);
//        moves.clear();
//        rew.push_back(0);
//        for (int i = 1; i < st[k].getPlace().size(); ++i) {
//            rew.push_back(0);
//            if ((st[k].place_[0].x == place_[i].x && st[k].place_[0].y == place_[i].y &&
//                 st[k].place_[i].x == place_[0].x &&
//                 st[k].place_[i].y == place_[0].y) ||
//                (st[k].place_[0].x == st[k].place_[i].x && st[k].place_[0].y == st[k].place_[i].y)) {
//                ++rew[0];
//                --rew[i];
//            }
//        }
//        vec.push_back(rew);
//        rew.clear();
//        int index;
//        std::vector<Observation> obs = std::vector<Observation>();
//        obs.emplace_back(0);
//        for(int i = 1; i < st[k].place_.size(); ++i) {
//            index = 0;
//            for (int l = 1; l < eight.size(); ++l) {
//                if (st[k].place_[0].x + eight[l].x == st[k].place_[i].x && st[k].place_[0].y + eight[l].y == st[k].place_[i].y) {
//                    index = l;
//                }
//            }
//            obs[0] = Observation(index,desc[index]);
//            obs.emplace_back(eight.size()-1-index,desc[eight.size()-1-index]);
//        }
//        obsvec.push_back(obs);
//        obs.clear();
//        PursuitOutcome p = PursuitOutcome(st.back(), obsvec.back(), vec.back());
//        pairs.emplace_back(p, st.back().prob_/prob_);
//    }
//    ProbDistribution prob(pairs);
//    return prob;
}



Outcome::Outcome(std::unique_ptr<State> &st, const std::vector<Observation> &ob, const std::vector<double> &rew):  ob_(ob), rew_(rew){
    st_ = std::move(st);
}


Domain::Domain(std::unique_ptr<State> &r)  {
    root_ = std::move(r);
}

std::string Domain::GetInfo() {}


const Outcome& ProbDistribution::GetRandom() { //TODO
    //std::vector<Pos> p = {{1,1}};
    return pairs_[3].first;
}


std::vector<Outcome> ProbDistribution::GetOutcomes() {
    std::vector<Outcome> list;// = std::vector<PursuitOutcome>(pairs_.size());
    for (auto pair : pairs_) {
        list.emplace_back(std::move(pair.first));//.push_back(std::move(pair.first));
    }
    return list;
}

ProbDistribution::ProbDistribution(const std::vector<std::pair<Outcome&, double>> &pairs): pairs_(pairs) {
}
