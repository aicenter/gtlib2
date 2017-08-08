//
// Created by rozliv on 02.08.2017.
//

#include "pursuit.h"


std::string PursuitAction::ToString() {
    return std::string();
}

PursuitAction::PursuitAction(int id) {

}


PursuitState::PursuitState(std::vector<Pos> &p):place_(p){
    probdis_ = {0.1,0.9};
}

PursuitState::PursuitState(std::vector<Pos> &p, double prob):place_(p), prob_(prob){
    probdis_ = {0.1,0.9};
}

std::vector<Action> PursuitState::getActions(int player) {
    std::vector<Action> list = std::vector<Action>();
    for (int i = 0; i < 5; ++i) {
        if((place_[player].x + m_[i].x) >= 0 && (place_[player].x + m_[i].x) < PursuitDomain::width_
           &&(place_[player].y + m_[i].y) >= 0 && (place_[player].y + m_[i].y) < PursuitDomain::height_){
            list.emplace_back(i, des_[i]);
        }
    }
    return list;
}

void PursuitState::getActions(std::vector<Action> &list,int player)const {
    for (int i = 0; i < 5; ++i) {
        if((place_[player].x + m_[i].x) >= 0 && (place_[player].x + m_[i].x) < PursuitDomain::width_
           &&(place_[player].y + m_[i].y) >= 0 && (place_[player].y + m_[i].y) < PursuitDomain::height_){
            list.emplace_back(i, des_[i]);
        }
    }

}

PursuitOutcome PursuitState::PerformAction(std::vector<Action>& actions)const {
    std::vector<Pos> moves = std::vector<Pos>(actions.size());
    double probability = prob_;
    for(int i = 0; i < actions.size();  ++i) {
        moves[i].x = m_[actions[i].getID()].x + place_[i].x;
        moves[i].y = m_[actions[i].getID()].y + place_[i].y;
        probability *= actions[i].getID() == 0 ? probdis_[0]: probdis_[1];
    }
    PursuitState s = PursuitState(moves, probability);
    std::vector<double> rew = std::vector<double>(s.place_.size());

    for (int i = 1; i < s.getPlace().size(); ++i) {
        if((s.place_[0].x == place_[i].x && s.place_[0].y == place_[i].y && s.place_[i].x == place_[0].x &&
            s.place_[i].y == place_[0].y) || (s.place_[0].x == s.place_[i].x && s.place_[0].y == s.place_[i].y)){
            ++rew[0];
            --rew[i];
        }
    }
    std::vector<Pos> eight = {{-2,-2},{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}, {0,0}};
    std::vector<std::string> desc = {"nowhere","top left", "top", "top right", "left", "right",
                                     "bottom left", "bottom", "bottom right", "same"};
    std::vector<Observation> obs = std::vector<Observation>();
    int index;
    obs.emplace_back(0);
    for(int i = 1; i < s.place_.size(); ++i) {
        index = 0;
        for (int j = 1; j < eight.size(); ++j) {
            if(s.place_[0].x + eight[j].x == s.place_[i].x && s.place_[0].y + eight[j].y == s.place_[i].y) {
                index = j;
            }
        }
        obs[0] = Observation(index,desc[index]);
        obs.emplace_back(eight.size()-1-index,desc[eight.size()-1-index]);
    }

    PursuitOutcome o = PursuitOutcome(s, obs, rew);
    return o;
}



PursuitOutcome::PursuitOutcome(const PursuitState &s, const std::vector<Observation> &ob,
                               const std::vector<double> &rew): st_(s), Outcome(ob, rew){}



PursuitDomain::PursuitDomain(int max, State &r): Domain(r),maxdepth_(max){}

int PursuitDomain::height_ = 0;
int PursuitDomain::width_ = 0;

std::string PursuitDomain::GetInfo() {
    return "Rozmery pole jsou: " + std::to_string(PursuitDomain::height_) + " x " +
            std::to_string(PursuitDomain::height_) + " maximalni hloubka grafu je: " +  std::to_string(maxdepth_) +
            " a pocatecni stav je: ...";
}

int count = 0;
std::vector<double> rewards;

void pursuit(PursuitDomain& domain, const PursuitState &state, int depth)
{
    for (int i = 0; i < domain.getMaxDepth()-depth; ++i) {
        cout << "    ";
    }

    string s = std::to_string(state.getPlace()[0].x) + " " + std::to_string(state.getPlace()[0].y) + "    " +
               std::to_string(state.getPlace()[1].x) + " " + std::to_string(state.getPlace()[1].y) + "   " +
            std::to_string(state.getPro());
    cout << s<<"   "<< domain.GetInfo() << '\n';
    vector<Action> actions = vector<Action>();
    vector<Action> actions2 = vector<Action>();
//    cout << "hloubka je: " << depth << "   ";
//    cout<< "1. " << state.getPlace()[0].x << " " << state.getPlace()[0].y;
//    cout<< "    2. " << state.getPlace()[1].x << " " << state.getPlace()[1].y << '\n';
    ++count;
    if(depth == 0){
        return;
    }

    state.getActions(actions,0);
    state.getActions(actions2,1);

    for (Action &action : actions) {
        for (Action &j : actions2) {
            vector<Action> base = {action, j};
            PursuitOutcome o = state.PerformAction(base);
            for (int i = 0; i < rewards.size(); ++i) {
                rewards[i] += o.getReward()[i];
            }
            pursuit(domain, o.getState(), depth - 1);
        }
    }
}
