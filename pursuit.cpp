//
// Created by rozliv on 02.08.2017.
//

#include "pursuit.h"

PursuitState::PursuitState(std::vector<Pos> &p):place(p){
    probDis = {0.1,0.9};
}

PursuitState::PursuitState(std::vector<Pos> &p, double t_prob):place(p), prob(t_prob){
    probDis = {0.1,0.9};
}

void PursuitState::getActions(std::vector<Action> &list,int player, int t_i, int t_j)const {
    for (int i = 0; i < 5; ++i) {
        if((place[player].x + m[i].x) >= 0 && (place[player].x + m[i].x) < t_j
           &&(place[player].y + m[i].y) >= 0 && (place[player].y + m[i].y) < t_i){
            list.emplace_back(i, des[i]);
        }
    }
}

PursuitOutcome PursuitState::PerformAction(std::vector<Action>& t_actions)const {
    std::vector<Pos> moves = std::vector<Pos>(t_actions.size());
    double probability = prob;
    for(int i = 0; i < t_actions.size();  ++i) {
        moves[i].x = m[t_actions[i].getID()].x + place[i].x;
        moves[i].y = m[t_actions[i].getID()].y + place[i].y;
        probability *= t_actions[i].getID() == 0 ? probDis[0]: probDis[1];
    }
    PursuitState s = PursuitState(moves, probability);
    std::vector<int> rew = std::vector<int>(s.place.size());

    for (int i = 1; i < s.getPlace().size(); ++i) {
        if((s.place[0].x == place[i].x && s.place[0].y == place[i].y && s.place[i].x == place[0].x && s.place[i].y == place[0].y) ||
           (s.place[0].x == s.place[i].x && s.place[0].y == s.place[i].y)){
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
    for(int i = 1; i < s.place.size(); ++i) {
        index = 0;
        for (int j = 1; j < eight.size(); ++j) {
            if(s.place[0].x + eight[j].x == s.place[i].x && s.place[0].y + eight[j].y == s.place[i].y) {
                index = j;
            }
        }
        obs[0] = Observation(index,desc[index]);
        obs.emplace_back(eight.size()-1-index,desc[eight.size()-1-index]);
    }

    PursuitOutcome o = PursuitOutcome(s, obs, rew);
    return o;
}



PursuitOutcome::PursuitOutcome(const PursuitState &s, const std::vector<Observation> &t_ob,
                               const std::vector<int> &t_rew): st(s), Outcome(t_ob, t_rew){}



PursuitDomain::PursuitDomain(int w, int h,int max, State &r): Domain(w,h,r), maxdepth(max){}


int count = 0;
std::vector<int> rewards;

void pursuit(PursuitDomain& domain, const PursuitState &state, int depth)
{
    for (int i = 0; i < domain.getMaxDepth()-depth; ++i) {
        cout << "    ";
    }

    string s = std::to_string(state.getPlace()[0].x) + " " + std::to_string(state.getPlace()[0].y) + "    " +
               std::to_string(state.getPlace()[1].x) + " " + std::to_string(state.getPlace()[1].y) + "   " +
            std::to_string(state.getPro());
    cout << s<< '\n';
    vector<Action> actions = vector<Action>();
    vector<Action> actions2 = vector<Action>();
//    cout << "hloubka je: " << depth << "   ";
//    cout<< "1. " << state.getPlace()[0].x << " " << state.getPlace()[0].y;
//    cout<< "    2. " << state.getPlace()[1].x << " " << state.getPlace()[1].y << '\n';
    ++count;
    if(depth == 0){return;}

    state.getActions(actions,0, domain.getHeight(), domain.getWidth());
    state.getActions(actions2,1, domain.getHeight(), domain.getWidth());

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